#include "ResourceManager.h"
#include "Resource.h"
#include "../Core/Directories.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
#include "../Core/JobManager.h"
#include "../Core/TextSerializer.h"
#include "../Core/OSLayer.h"
namespace fs = std::experimental::filesystem;

namespace fv
{
    const char* g_CachedFiletimes = "CachedFiletimes.json";

    ResourceManager::~ResourceManager()
    {
        m_Closing = true;
        if ( m_ResourceThread.joinable() )
        {
            m_ResourceThread.join();
        }
    }

    void ResourceManager::cleanupResourcesWithoutReferences()
    {
        scoped_lock lk(m_NameToResourceMutex);
        for ( auto it = m_NameToResource.begin(); it != m_NameToResource.end(); )
        {
            if ( it->second.resource.use_count()==1 )
            {
                it = m_NameToResource.erase(it);
                continue;
            }
            ++it;
        }
    }

    void ResourceManager::initialize()
    {
        FV_CHECK_BG();
        readResourceConfig(m_Config);
        cacheSearchDirectories();
        cacheFiletimes();
        m_ResourceThread = Thread([this]()
        {
            OSSetThreadName("ResourcesLoad");
            loadThread();
        });
    }

    void ResourceManager::readResourceConfig(ResourceConfig& config)
    {
        FV_CHECK_BG();
        // TODO actually read from config
        config.loadThreadSleepTimeMs = 10;
    }

    void ResourceManager::cacheSearchDirectories()
    {
        FV_CHECK_BG();
        for ( auto& pathIt : fs::recursive_directory_iterator(Directories::assets().c_str()) )
        {
            const Path& dir = pathIt.path();
            Path filename = dir.filename();
            auto fIt = m_CachedFilenameToDirectories.find(filename);
            if ( fIt == m_CachedFilenameToDirectories.end() )
            {
                m_CachedFilenameToDirectories[filename] = dir;
            }
            else
            {
                LOGW("Multiple resources with the name '%s' found. '%s' and '%s'. All must have unique name.",
                     filename.string().c_str(), dir.string().c_str(), m_CachedFilenameToDirectories[filename].string().c_str());
            }
        }
    }

    void ResourceManager::cacheFiletimes()
    {
        FV_CHECK_BG();
        Path chachedFiletimes = Directories::intermediate() / g_CachedFiletimes;
        TextSerializer ts(chachedFiletimes.string().c_str());
        if ( !ts.hasSerializeErrors() )
        {
            ts.serializeMap("filetimes", m_CachedFiletimes);
        }
        else
        {
            LOGW("Failed to load %s. All assets will reimport themselves. May take a long time...", chachedFiletimes.string().c_str());
        }
    }

    Path ResourceManager::filenameToDirectory(const String& name) const
    {
        auto fIt = m_CachedFilenameToDirectories.find(name);
        if ( fIt == m_CachedFilenameToDirectories.end() )
        {
            return "";
        }
        return fIt->second;
    }

    M<Resource> ResourceManager::findOrCreateResource(const String& filename, u32 type, bool& wasAlreadyCreated)
    {
        // See if already exists
        wasAlreadyCreated = false;
        scoped_lock lk(m_NameToResourceMutex);
        auto rIt = m_NameToResource.find(filename);
        if ( rIt != m_NameToResource.end() )
        {
            wasAlreadyCreated = true;
            return rIt->second.resource;
        }
        const TypeInfo* ti = typeManager()->typeInfo(type);
        if ( !ti ) return nullptr;
        M<Resource> resource = M<Resource>(sc<Resource*>(ti->createFunc(1)));
        if (!resource) return nullptr; // Type to static create function failed.
        m_NameToResource[filename] = { resource, filenameToDirectory(filename) }; // Store while having lock.
        return resource;
    }

    u64 ResourceManager::getAndUpdateCachedFiletime(const String& filename, u64 newDiskTime, bool& fileTimesUpdated)
    {
        u64 oldTime = -1;
        auto cIt = m_CachedFiletimes.find( filename );
        if ( cIt != m_CachedFiletimes.end() )
        {
            oldTime = cIt->second;
            cIt->second = newDiskTime;
            fileTimesUpdated = true;
        }
        else if ( newDiskTime != -1 ) // Do not store invalid file time
        {
            m_CachedFiletimes[filename] = newDiskTime;
            fileTimesUpdated = true;
        }
        return oldTime;
    }

    void ResourceManager::writeCachedFiletimes()
    {
        TextSerializer ts;
        ts.serializeMap("filetimes", m_CachedFiletimes);
        if ( !ts.hasSerializeErrors() )
        {
            ts.writeToFile((Directories::intermediate() / g_CachedFiletimes).string().c_str());
        }
        else
        {
            LOGW("Failed to write file times file %s.", g_CachedFiletimes);
        }
    }

    void ResourceManager::loadThread()
    {
        while ( !m_Closing )
        {
            // Copy list to avoid main thread to await the expensinve FileTime function while having lock.
            m_LoadedResourcesCopy.clear();
            {
                scoped_lock lk(m_NameToResourceMutex);
                m_LoadedResourcesCopy.reserve( m_NameToResource.size() );
                for ( auto& kvp : m_NameToResource )
                {
                    LoadedResourceInfo lri = kvp.second;
                    kvp.second.loaded = true;
                    m_LoadedResourcesCopy.emplace_back( lri );
                }
            }

            // Check to see if files must reimport (or just load if is first time).
            bool fileTimesWereUpdated = false;
            for ( auto& rsi : m_LoadedResourcesCopy )
            {
                if ( rsi.path.empty() ) continue; // Skip not found resources
                u64 diskFiletime   = FileModifiedTime( rsi.path.string().c_str() );
                u64 cachedFiletime = getAndUpdateCachedFiletime( rsi.path.filename().string(), diskFiletime, fileTimesWereUpdated );
                bool reimport = ( diskFiletime != cachedFiletime || cachedFiletime == -1);
                if ( reimport || !rsi.loaded )
                {
                    ResourceToLoad rtl = { rsi.resource, rsi.path, reimport };
                    rsi.resource->load( rtl );
                }
            }

            // If any file times were updated, write back cached file. So that next
            // startup of engine, changes in mean time can be detected against cached file.
            if ( fileTimesWereUpdated )
            {
                writeCachedFiletimes();
            }

            // Wait for next iteration
            Suspend( m_Config.loadThreadSleepTimeMs *.001 );
        }
    }

    ResourceManager* g_ResourceManager {};
    ResourceManager* resourceManager() { return CreateOnce(g_ResourceManager); }
    void deleteResourceManager() { delete g_ResourceManager; g_ResourceManager=nullptr; }
}
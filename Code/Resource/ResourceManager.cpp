#include "ResourceManager.h"
#include "Resource.h"
#include "../Core/Directories.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
#include "../Core/JobManager.h"
#include "../Core/TextSerializer.h"
#include "../Core/OSLayer.h"
namespace fs = std::filesystem;

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

    FV_TS void ResourceManager::cleanupResourcesWithoutReferences()
    {
        Lock lk(m_ResourcesMutex);
        for ( auto it = m_Resources.begin(); it != m_Resources.end(); )
        {
            if ( it->second->resource.use_count()==1 )
            {
                it = m_Resources.erase(it);
                continue;
            }
            ++it;
        }
    }

    FV_BG void ResourceManager::initialize()
    {
        FV_CHECK_BG();
        readResourceConfig(m_Config);
        cacheFiletimes( true );
        m_ResourceThread = Thread([this]()
        {
            OSSetThreadName("ResourcesLoad");
            loadThread();
        });
    }

    FV_BG void ResourceManager::readResourceConfig(ResourceConfig& config)
    {
        FV_CHECK_BG();
        // TODO actually read from config
        config.loadThreadSleepTimeMs = 10;
        config.autoCleanupUnreferencedAssets = false;
    }

    void ResourceManager::cacheFiletimes(bool isRead)
    {
        if ( isRead )
        {
            FV_CHECK_BG();
        }
        Path chachedFiletimes = Directories::intermediate() / g_CachedFiletimes;
        TextSerializer ts( chachedFiletimes.string().c_str(), isRead );
        ts.serializeMap( "filetimes", m_CachedFiletimes );
        if ( !isRead ) ts.flushWrites();
        if ( ts.hasSerializeErrors() )
        {
            if ( isRead )
            {
                LOGW( "Failed to read %s. All assets will reimport (slow process).", chachedFiletimes.c_str() );
            }
            else
            {
                LOGW( "Failed to write filetimes to %s.", chachedFiletimes.c_str() );
            }
        }
    }

    FV_TS M<Resource> ResourceManager::findOrCreateResource(const String& path, u32 type, bool& wasAlreadyCreated)
    {
        // See if already exists
        {
            wasAlreadyCreated = false;
            Lock lk(m_ResourcesMutex);
            auto rIt = m_Resources.find(path);
            if ( rIt != m_Resources.end() )
            {
                wasAlreadyCreated = true;
                return rIt->second->resource;
            }
        }
        // Did not exist, get constructor from type.
        const TypeInfo* ti = typeManager()->typeInfo(type);
        if ( !ti ) 
        {
            LOGW( "Failed to find %s.", path.c_str() );
            return nullptr;
        }
        // Try construct new resource.
        M<Resource> resource = M<Resource>(sc<Resource*>(ti->createFunc(1)));
        if (!resource) return nullptr; // Type to static create function failed.
        auto rsi = make_shared<LoadedResourceInfo>();
        rsi->loadedOnce = false;
        rsi->path = Directories::assets() / path;
        rsi->resource = resource;
        resource->m_Filename = path;
        Lock lk(m_ResourcesMutex);
        m_Resources[path] = rsi; // Store while having lock.
        return resource;
    }

    u64 ResourceManager::getAndUpdateCachedFiletime(const String& path, u64 newDiskTime, bool& fileTimesUpdated)
    {
        assert(newDiskTime != -1);
        if ( newDiskTime==-1 ) return -1; // Do not allow to store invalid disk time.
        u64 oldTime = -1;
        fileTimesUpdated = false;
        auto cIt = m_CachedFiletimes.find( path );
        if ( cIt != m_CachedFiletimes.end() )
        {
            oldTime = cIt->second;
            cIt->second = newDiskTime;
            fileTimesUpdated = newDiskTime != oldTime;
        }
        else
        {
            m_CachedFiletimes[path] = newDiskTime;
            fileTimesUpdated = true;
        }
        return oldTime;
    }

    void ResourceManager::loadThread()
    {
        Vector<M<LoadedResourceInfo>> resourcesCopy;

        while ( !m_Closing )
        {
            // Copy list to avoid main thread to await the expensinve FileTime function while having lock.
            {
                Lock lk(m_ResourcesMutex);
                for ( auto& kvp : m_Resources )
                {
                    auto lri = kvp.second;
                    resourcesCopy.emplace_back( lri );
                }
            }

            // Check to see if files must reimport (or just load if is first time).
            bool fileTimesWereUpdated = false;
            for ( auto& rsi : resourcesCopy )
            {
                if ( rsi->path.empty() ) continue; // Skip not found resources
                u64 diskFiletime   = FileModifiedTime( rsi->path.string().c_str() );
                bool fileTimeWasUpdated = false;
                u64 cachedFiletime = getAndUpdateCachedFiletime( rsi->path.string(), diskFiletime, fileTimeWasUpdated );
                if ( fileTimeWasUpdated && !fileTimesWereUpdated ) fileTimesWereUpdated = true;
                bool reimport = ( diskFiletime != cachedFiletime || cachedFiletime == -1 );
                if ( reimport || !rsi->loadedOnce )
                {
                    ResourceToLoad rtl = { rsi->resource, rsi->path, reimport };
                    rsi->resource->load_RT( rtl );
                    rsi->loadedOnce = true;
                }
            }
            resourcesCopy.clear();

            // If tile time was changed, write the new time so that when engine restarts all files
            // that were changed in the mean time can be detected. As such we are not dealing
            // with outdated 'generated' intermediate assets.
            if ( fileTimesWereUpdated )
            {
                cacheFiletimes( false );
            }

            // Wait for next iteration.
            Suspend( m_Config.loadThreadSleepTimeMs *.001 );

            // Cleanup unreferenced assets.
            if ( m_Config.autoCleanupUnreferencedAssets )
            {
                cleanupResourcesWithoutReferences();
            }
        }
    }

    ResourceManager* g_ResourceManager {};
    ResourceManager* resourceManager() { return CreateOnce(g_ResourceManager); }
    void deleteResourceManager() { delete g_ResourceManager; g_ResourceManager=nullptr; }
}
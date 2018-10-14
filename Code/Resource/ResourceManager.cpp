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

    ResourceManager::ResourceManager()
    {
        // Cache all search directories for resources.
        for ( auto& pathIt : fs::recursive_directory_iterator( Directories::assets().c_str() ) )
        {
            Path dir = pathIt.path();
            Path filename = dir.filename();
            dir.remove_filename();
            auto fIt = m_FilenameToDirectory.find( filename );
            if ( fIt == m_FilenameToDirectory.end() )
            {
                m_FilenameToDirectory[ filename ] = dir;
            }
            else
            {
                LOGW("Multiple resources with the name '%s' found. '%s' and '%s'. All must have unique name.", 
                     filename.string().c_str(), dir.string().c_str(), m_FilenameToDirectory[ filename ].string().c_str() );
            }
        }

        // Read file_names to file_times to detect changes that have occurred since last start.
        Path chachedFiletimes = Directories::intermediate() / g_CachedFiletimes;
        TextSerializer ts( chachedFiletimes.string().c_str() );
        if ( !ts.hasSerializeErrors() )
        {
            ts.serializeMap( "filetimes", m_CachedFiletimes );
        }
        else
        {
            LOGW("Failed to load %s. All assets will reimport themselves. May take a long time...", chachedFiletimes.string().c_str() );
        }

        readResourceConfig( m_Config );

        m_ResourceThread = Thread( [this]()
        {
            OSSetThreadName("ResourcesLoad");
            loadThread();
        });

        m_FiletimesThread = Thread( [this]()
        {
            OSSetThreadName("CachedFiletimes");
            fileTimesThread();
        });

    }

    ResourceManager::~ResourceManager()
    {
        m_Closing = true;
        if ( m_ResourceThread.joinable() )
        {
            m_ResourceThread.join();
        }
        if ( m_FiletimesThread.joinable() )
        {
            m_FiletimesThread.join();
        }
    }

    M<Resource> ResourceManager::load(u32 type, const String& name)
    {
        Path loadPath;
        M<Resource> resource = findOrCreateResource( name, type, loadPath );
        if ( !resource ) 
        {
            // Should always succeed.
            return nullptr;
        }
        // Add to list of pending resources to be loaded
        {
            ResourceToLoad rtl = { resource, loadPath / name, false };
            scoped_lock lk (m_PendingListMutex);
            m_PendingResourcesToLoad[ m_ListToFill ].emplace_back( rtl );
        }
        return resource;
    }

    void ResourceManager::readResourceConfig(ResourceConfig& config)
    {
        FV_CHECK_MO();

        // TODO actually read from config
        config.loadThreadSleepTimeMs = 10;
    }

    M<Resource> ResourceManager::findOrCreateResource(const String& name, u32 type, Path& loadPath)
    {
        // See if already exists
        scoped_lock lk(m_NameToResourceMutex);
        auto rIt = m_NameToResource.find(name);
        if ( rIt != m_NameToResource.end() )
        {
            return rIt->second;
        }
        // Try find from name to directory.
        auto fIt = m_FilenameToDirectory.find(name);
        if ( fIt == m_FilenameToDirectory.end() )
        {
            LOGW("No resource with name %s found.", name.c_str());
            return nullptr;
        }
        loadPath = fIt->second;
        const TypeInfo* ti = typeManager()->typeInfo(type);
        if ( !ti ) return nullptr;
        M<Resource> resource = M<Resource>(sc<Resource*>(ti->createFunc(1)));
        if (!resource) return nullptr; // Type to static create function failed.
        m_NameToResource[name] = resource; // Store while having lock.
        return resource;
    }

    bool ResourceManager::shouldReimport(const Path& path)
    {
        u64 modifiedTime = FileModifiedTime( path.string().c_str() );
        scoped_lock lk(m_CachedFiletimesMutex);
        auto it = m_CachedFiletimes.find( path.filename().string() );
        if ( it == m_CachedFiletimes.end() || it->second != modifiedTime )
        {
            if ( modifiedTime != -1 ) // Only store valid times. If file did not exist or open failure -1 is returned.
            {
                m_CachedFiletimes[ path.filename().string() ] = modifiedTime;
            }
            else
            {
                LOGW("Could not obtain file time for %s.", path.string().c_str());
            }
            return true;
        }
        return false;
    }

    void ResourceManager::cleanupResourcesWithoutReferences()
    {
        scoped_lock lk(m_NameToResourceMutex);
        for ( auto it = m_NameToResource.begin(); it != m_NameToResource.end(); )
        {
            if ( it->second.use_count()==1 )
            {
                it = m_NameToResource.erase( it );
                continue;
            }
            ++it;
        }
    }

    void ResourceManager::loadThread()
    {
        while ( !m_Closing )
        {
            // Swap list indexes of pending resources with empty list.
            {
                scoped_lock lk(m_PendingListMutex);
                assert( m_PendingResourcesToLoad[ m_StuffedList ].size()==0 );
                std::swap( m_StuffedList, m_ListToFill );
            }
            // Do not load in parallel as that could mix up the requested loading order.
            for ( auto& rsl : m_PendingResourcesToLoad[m_StuffedList] )
            {
                rsl.reimport = shouldReimport( rsl.loadPath );
                rsl.resource->load( rsl );
            }
            m_PendingResourcesToLoad[ m_StuffedList ].clear();
            // Wait for next iteration
            Suspend( m_Config.loadThreadSleepTimeMs *.001 );
        }
    }

    void ResourceManager::fileTimesThread()
    {
        while ( !m_Closing )
        {
            Map<String, u64> cachedFiletimesCpy;
            {
                scoped_lock lk(m_CachedFiletimesMutex);
                cachedFiletimesCpy = m_CachedFiletimes;
            }

            TextSerializer ts;
            ts.serializeMap( "filetimes", cachedFiletimesCpy );
            if ( !ts.hasSerializeErrors() )
            {
                ts.writeToFile( (Directories::intermediate() / g_CachedFiletimes).string().c_str() );
            }
            else
            {
                LOGW( "Failed to write file times file %s.", g_CachedFiletimes );
            }

            // Wait for next iteration
            Suspend( m_Config.writeCachedFiletimesSleepMs * .001 );
        }
    }

    ResourceManager* g_ResourceManager {};
    ResourceManager* resourceManager() { return CreateOnce(g_ResourceManager); }
    void deleteResourceManager() { delete g_ResourceManager; g_ResourceManager=nullptr; }
}
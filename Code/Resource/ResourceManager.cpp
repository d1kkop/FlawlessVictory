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

    FV_TS void ResourceManager::cleanupResourcesWithoutReferences()
    {
        Lock lk( m_ResourcesMutex );
        for ( auto it = m_Resources.begin(); it != m_Resources.end(); )
        {
            if ( it->second->resource.use_count()==1 )
            {
                it = m_Resources.erase( it );
                continue;
            }
            ++it;
        }
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
                LOGW( "%s not found. All assets will reimport (slow process). If this is the first run, then this is expected behaviour.", chachedFiletimes.string().c_str() );
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
        rsi->diskPath  = Directories::assets() / path;
        rsi->assetPath = path;
        rsi->resource  = resource;
        resource->m_Filename = path;
        Lock lk(m_ResourcesMutex);
        m_Resources[path] = rsi; // Store while having lock.
        return resource;
    }

    u64 ResourceManager::getAndUpdateCachedFiletime(const String& path, u64 newDiskTime, bool& fileTimesUpdated)
    {
        fileTimesUpdated = false;
        if ( newDiskTime==-1 ) return -1; // Do not allow to store invalid disk time.
        u64 oldTime = -1;
        auto cIt = m_CachedFiletimes.find( path );
        if ( cIt != m_CachedFiletimes.end() )
        {
            oldTime = cIt->second;
            cIt->second = newDiskTime;
        }
        else
        {
            m_CachedFiletimes[path] = newDiskTime;
        }
        // FiletimesUpdated means that it was written to the list, the newDiskTime might however be the same as the oldTime.
        fileTimesUpdated = true;
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
                // Do no disk access if we already know that this path does not exists.
                if ( rsi->triedLoadOnce && !rsi->pathExists )
                {
                    continue;
                }
                if ( !FileExists(rsi->diskPath.string().c_str()) )
                {
                    LOGW( "Failed to find disk path %s. Cannot load resource.", rsi->diskPath.string().c_str() );
                    // Mark tried to load it once, this file cannot be found, do no longer attempt to find it.
                    rsi->triedLoadOnce = true;
                    rsi->pathExists = false;
                    continue;
                }
                u64 diskFiletime = FileModifiedTime( rsi->diskPath.string().c_str() );
                bool fileTimeWasUpdated = false;
                u64 cachedFiletime = getAndUpdateCachedFiletime( rsi->assetPath.string(), diskFiletime, fileTimeWasUpdated );
                if ( fileTimeWasUpdated && !fileTimesWereUpdated ) fileTimesWereUpdated = true;
                bool reimport = ( diskFiletime != cachedFiletime || cachedFiletime == -1 );
                if ( reimport || !rsi->triedLoadOnce )
                {
                    ResourceToLoad rtl = { rsi->resource, rsi->diskPath, reimport };
                    rsi->resource->load_RT( rtl );
                    rsi->triedLoadOnce = true;
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
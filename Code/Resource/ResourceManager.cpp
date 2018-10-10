#include "ResourceManager.h"
#include "Resource.h"
#include "../Core/Directories.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
#include "../Core/JobManager.h"
namespace fs = std::experimental::filesystem;

namespace fv
{
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
                LOGW("Multiple resource with the name '%s' found. In '%s' and '%s'. All must have unique name.", 
                     filename.string().c_str(), dir.string().c_str(), m_FilenameToDirectory[ filename ].string().c_str() );
            }
        }

        readResourceConfig( m_Config );

        m_ResourceThread = Thread( [this]()
        {
            loadThread();
        });
    }

    ResourceManager::~ResourceManager()
    {
        m_Closing = true;
        if ( m_ResourceThread.joinable() )
        {
            m_ResourceThread.join();
        }
    }

    M<Resource> ResourceManager::load(u32 type, const String& name)
    {
        scoped_lock lk(m_LoadMutex);
        // See if resource was previously loaded
        auto rIt = m_NameToResource.find( name );
        if ( rIt != m_NameToResource.end() )
        {
            return rIt->second;
        }
        auto fIt = m_FilenameToDirectory.find( name );
        if ( fIt == m_FilenameToDirectory.end() )
        {
            LOGW("No resource with name %s found.", name.c_str());
            return nullptr;
        }
        // Obtaining typeInfo is thread safe. All types are registered before main entry.
        const TypeInfo* ti = typeManager()->typeInfo(type);
        if (!ti) return nullptr;
        M<Resource> resource = M<Resource>( sc<Resource*>( ti->createFunc(1) ) );
        if ( !resource ) return nullptr;
        // Store already, although not loaded yet. Other requests to same resource should obtain this handle.
        m_NameToResource[name] = resource;
        // Add to list of pending resources to be loaded
        ResourceToLoad rtl = { resource, fIt->second / name };
        m_PendingResourcesToLoad[ m_ListToFill ].emplace_back( rtl );
        return resource;
    }

    void ResourceManager::readResourceConfig(ResourceConfig& config)
    {
        // TODO actually read from config
        config.loadThreadSleepTimeMs = 10;
    }

    void ResourceManager::cleanupResourcesWithoutReferences()
    {
        scoped_lock lk(m_LoadMutex);
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
            // Swap list of pending resources with empty list.
            {
                scoped_lock lk(m_LoadMutex);
                assert( m_PendingResourcesToLoad[ m_StuffedList ].size()==0 );
                std::swap( m_StuffedList, m_ListToFill );
            }
            // Do not load in parallel as that could mix up the requested loading order. load as requested.
            for ( auto& rsl : m_PendingResourcesToLoad[m_StuffedList] )
            {
                rsl.resource->load( rsl );
            }
            m_PendingResourcesToLoad[ m_StuffedList ].clear();
            // Wait for next iteration
            Suspend( m_Config.loadThreadSleepTimeMs*1000 );
        }
    }

    ResourceManager* g_ResourceManager {};
    ResourceManager* resourceManager() { return CreateOnce(g_ResourceManager); }
    void deleteResourceManager() { delete g_ResourceManager; g_ResourceManager=nullptr; }
}
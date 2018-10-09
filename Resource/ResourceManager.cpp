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
    }

    M<Resource> ResourceManager::load(u32 type, const String& name)
    {
        Path path;
        M<Resource> resource;
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
            path = fIt->second;
            const TypeInfo* ti = typeManager()->typeInfo(type);
            if (!ti) return nullptr;
            resource = M<Resource>( sc<Resource*>( ti->createFunc(1) ) );
            if ( !resource ) return nullptr;
            // Store already, although not loaded yet. Other requests to same resource should already obtain this handle.
            m_NameToResource[name] = resource;
        }
        jobManager()->addJob([=]() mutable
        {
            resource->load( path / name );
        }, [=](Job* j) 
        {
            resource->onDoneOrCancelled(j);
        });
        return resource;
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

    ResourceManager* g_ResourceManager {};
    ResourceManager* resourceManager() { return CreateOnce(g_ResourceManager); }
    void deleteResourceManager() { delete g_ResourceManager; g_ResourceManager=nullptr; }
}
#pragma once
#include "../Core/Common.h"

namespace fv
{
    class Resource;

    struct ResourceToLoad
    {
        M<Resource> resource;
        Path loadPath;
    };

    class ResourceManager
    {
    public:
        ResourceManager();

        FV_TS FV_DLL M<Resource> load(u32 type, const String& name);
        FV_TS FV_DLL void cleanupResourcesWithoutReferences();
        FV_MO FV_DLL void processResources();

        template <class T> FV_TS M<T> load(const String& name);

    private:
        // Note resources are not recycled but shared, so to have a shared ptr.
        Map<Path, M<Resource>> m_NameToResource;
        Map<Path, Path> m_FilenameToDirectory;
        Vector<ResourceToLoad> m_PendingResourcesToLoad[2];
        u32 m_PendingListToFill = 0;
        u32 m_PendingListToLoad = 1;
        Mutex m_LoadMutex;
    };


    template <class T>
    M<T> ResourceManager::load(const String& name)
    {
        return std::static_pointer_cast<T>( load(T::type(), name) );
    }

    FV_DLL ResourceManager* resourceManager();
    FV_DLL void deleteResourceManager();
}

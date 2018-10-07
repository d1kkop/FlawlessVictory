#pragma once
#include "../Core/Common.h"

namespace fv
{
    class Resource;

    class ResourceManager
    {
    public:
        ResourceManager();

        FV_TS FV_DLL M<Resource> load(u32 type, const String& name);
        FV_TS FV_DLL void cleanupResourcesWithoutReferences();

        template <class T> FV_TS M<T> load(const String& name);

    private:
        // Note resources are not recycled but shared, so to have a shared ptr.
        Map<Path, M<Resource>> m_NameToResource;
        Map<Path, Path> m_FilenameToDirectory;
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

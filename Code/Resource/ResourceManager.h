#pragma once
#include "../Core/Common.h"

namespace fv
{
    class Resource;

    struct ResourceConfig
    {
        u32 loadThreadSleepTimeMs = 200;
    };

    struct ResourceToLoad
    {
        M<Resource> resource;
        Path loadPath;
        bool reimport;
    };

    struct LoadedResourceInfo
    {
        M<Resource> resource;
        Path path;
        bool loaded;
    };

    class ResourceManager
    {
    public:
        ~ResourceManager();
        FV_TS FV_DLL void cleanupResourcesWithoutReferences();

        template <class T>
        FV_TS M<T> load(const String& filename);

        template <class T>
        FV_TS M<T> create(const String& filename, bool& wasAlreadyCreated);

    private:
        FV_BG FV_DLL void initialize(); 
        FV_BG void readResourceConfig(ResourceConfig& config);
        FV_BG void cacheSearchDirectories();
        FV_BG void cacheFiletimes();
        FV_TS Path filenameToDirectory(const String& filename) const; // Thread safe because directories are cached on startup.
        FV_TS FV_DLL M<Resource> findOrCreateResource(const String& filename, u32 type, bool& wasAlreadyCreated);
        u64  getAndUpdateCachedFiletime( const String& filename, u64 newDiskTime, bool& fileTimesUpdated );
        void writeCachedFiletimes();
        void loadThread();

        Map<Path, LoadedResourceInfo> m_NameToResource;
        Vector<LoadedResourceInfo> m_LoadedResourcesCopy;
        Map<Path, Path> m_CachedFilenameToDirectories;
        Map<String, u64> m_CachedFiletimes;
        Thread m_ResourceThread;
        Mutex m_NameToResourceMutex;
        Mutex m_ResourcesNotCopiedMutex;
        ResourceConfig m_Config{};
        Atomic<bool> m_Closing = false;

        friend class SystemManager;
    };


    template <class T>
    M<T> ResourceManager::load(const String& filename)
    {
        bool wasAlreadyCreated;
        return std::static_pointer_cast<T>( findOrCreateResource(filename, T::type(), wasAlreadyCreated) );
    }

    template <class T>
    M<T> ResourceManager::create(const String& filename, bool& wasAlreadyCreated)
    {
        return std::static_pointer_cast<T>( findOrCreateResource(filename, T::type(), wasAlreadyCreated) );
    }

    FV_DLL ResourceManager* resourceManager();
    FV_DLL void deleteResourceManager();

    template <class T>
    M<T> Load(const String& filename)
    {
        return resourceManager()->load<T>(filename);
    }
}

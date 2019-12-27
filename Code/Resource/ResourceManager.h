#pragma once
#include "../Core/Common.h"

namespace fv
{
    class Resource;

    struct ResourceConfig
    {
        u32 loadThreadSleepTimeMs = 200;
        bool autoCleanupUnreferencedAssets = true;
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
        bool loadedOnce;
    };

    /*  The ResourceManager converts/loads raw resources to binary (generated) resources.
        It keeps track of filetimes so that a consequative engine startup detects 
        changed raw resources.
        It also auto reloads resources when the engine is running and raw assets are changed. */
    class ResourceManager
    {
    public:
        ~ResourceManager();

    private:
        FV_BG FV_DLL void initialize(); 
        FV_BG void readResourceConfig(ResourceConfig& config);

    public:
        FV_TS FV_DLL void cleanupResourcesWithoutReferences();

        // Path must be relative to Assets folder.
        template <class T>
        FV_TS M<T> load(const String& path);

        template <class T>
        FV_TS M<T> create(const String& path, bool& wasAlreadyCreated);

    private:
        FV_TS FV_DLL M<Resource> findOrCreateResource(const String& path, u32 type, bool& wasAlreadyCreated);

        // Writes or reads cached filetimes. Each raw resource has an associated filetime.
        void cacheFiletimes(bool isRead);

        // Sets new filetime and returns old (if existed, else -1 is returned).
        u64  getAndUpdateCachedFiletime( const String& path, u64 newDiskTime, bool& fileTimesUpdated );
        void loadThread();

        Map<Path, M<LoadedResourceInfo>> m_Resources;
        Map<String, u64> m_CachedFiletimes;
        Thread m_ResourceThread;
        Mutex  m_ResourcesMutex;
        ResourceConfig m_Config{};
        Atomic<bool> m_Closing = false;

        friend class SystemManager;
    };


    template <class T>
    FV_TS M<T> ResourceManager::load(const String& path)
    {
        bool wasAlreadyCreated;
        return spc<T>( findOrCreateResource(path, T::type(), wasAlreadyCreated) );
    }

    template <class T>
    FV_TS M<T> ResourceManager::create(const String& path, bool& wasAlreadyCreated)
    {
        return spc<T>( findOrCreateResource(path, T::type(), wasAlreadyCreated) );
    }

    FV_DLL ResourceManager* resourceManager();
    FV_DLL void deleteResourceManager();

    template <class T>
    FV_TS M<T> Load(const String& filename)
    {
        return resourceManager()->load<T>(filename);
    }
}

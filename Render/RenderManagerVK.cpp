#include "RenderManagerVK.h"
#if FV_VULKAN
#include "GraphicsResourceVK.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
#include "../Core/ComponentManager.h"

namespace fv
{

    RenderManagerVK::~RenderManagerVK()
    {
        closeGraphics();
    }

    bool RenderManagerVK::initGraphics(const RenderManagerParams& params)
    {
        m_CreateParams = params;
        obtainExtensions();
        obtainLayers();
        if ( !createIntance()) return false;
        if ( !createSurfaces() ) return false;
        trySetupDebugCallback();
        if ( !createDevices() ) return false;
        LOG("VK Initialized succesful.");
        return true;
    }

    void RenderManagerVK::closeGraphics()
    {
        // Remove debug message handler
        auto destroyDebugUtilMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
        if ( destroyDebugUtilMessenger && m_DebugCallback )
        {
            destroyDebugUtilMessenger( m_Instance, m_DebugCallback, nullptr );
        }
        vkDestroySurfaceKHR(m_Instance, m_WindowSurface, nullptr);
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = nullptr;
        for ( auto* c : m_Extensions) delete [] c;
        for ( auto* c : m_Layers) delete [] c;
    }

    GraphicResource* RenderManagerVK::createGraphic()
    {
        // Recycle GraphicResources.
        GraphicResource* graphic = sc<GraphicResource*>(componentManager()->newComponent(GraphicResourceVK::type()));
        return graphic;
    }

    void RenderManagerVK::obtainExtensions()
    {
        uint32_t extensionCount = 0;
        Vector<VkExtensionProperties> extensions;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        extensions.resize(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        for ( u32 i=0; i< extensionCount; ++i )
        {
            const char* extension = NewString(extensions[i].extensionName);
            m_Extensions.emplace_back(extension);
        }
    }

    void RenderManagerVK::obtainLayers()
    {
        uint32_t layerCount;
        Vector<VkLayerProperties> layers;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        layers.resize(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
        for ( u32 i = 0; i < layerCount; i++ )
        {
            // TODO add more required layers
            if ( strcmp(layers[i].layerName, "VK_LAYER_LUNARG_standard_validation")== 0 )
            {
                const char* layer = NewString(layers[i].layerName);
                m_Layers.emplace_back(layer);
            }
        }
    }

    bool RenderManagerVK::createIntance()
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = 1;
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = 1;
        appInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = (u32)m_Extensions.size();
        createInfo.ppEnabledExtensionNames = m_Extensions.data();
    #if FV_DEBUG
        createInfo.enabledLayerCount = (u32)m_Layers.size();// layerCount; // TODO check why with layers createInstace fails
        createInfo.ppEnabledLayerNames = m_Layers.data();
    #endif

        VkResult res = vkCreateInstance( &createInfo, nullptr, &m_Instance );
        if (res != VK_SUCCESS )
        {
            LOGC("VK Create instance failed.");
            return false;
        }

        return true;
    }

    bool RenderManagerVK::trySetupDebugCallback()
    {
    #if !FV_DEBUG
        return false;
    #endif
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = 
   //         VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
   //         VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr; // Optional
        auto createDebugUtilsMesgenger = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
        if ( !createDebugUtilsMesgenger || 
             createDebugUtilsMesgenger( m_Instance, &createInfo, nullptr, &m_DebugCallback ) != VK_SUCCESS )
        {
            LOGW( "VK Setup debug callback failed." );
            return false;
        }
        return true;
    }

    bool RenderManagerVK::createDevices()
    {
        uint32_t deviceCount = 0;
        Vector<VkPhysicalDevice> devices;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        devices.resize( deviceCount );
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        // For each physical device
        for ( auto& dv : devices )
        {
            // Check if is suitable device
            DeviceVK suitableDevice {};
            QueueFamilyIndicesVK queueIndices{};
            if ( isDeviceSuitable( dv, suitableDevice.name, queueIndices ) )
            {
                assert( queueIndices.complete() );
                float priorities = 1.f;
                Set<u32> uniqueQueueIndices = { *queueIndices.graphics, *queueIndices.compute, *queueIndices.transfer, *queueIndices.sparse };
                Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
                for ( auto& uIdx : uniqueQueueIndices )
                {
                    VkDeviceQueueCreateInfo dqci {};
                    dqci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    dqci.queueCount = 1;
                    dqci.queueFamilyIndex = uIdx;
                    dqci.pQueuePriorities = &priorities;
                    queueCreateInfos.emplace_back( dqci );
                }

                VkPhysicalDeviceFeatures deviceFeatures = {};
                VkDeviceCreateInfo createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                createInfo.pQueueCreateInfos = queueCreateInfos.data();
                createInfo.queueCreateInfoCount = (u32)uniqueQueueIndices.size();
                createInfo.pEnabledFeatures  = &deviceFeatures;
                createInfo.enabledLayerCount = (u32)m_Layers.size();
                createInfo.ppEnabledLayerNames = m_Layers.data();

                suitableDevice.physicalDevice = dv;
                auto res = vkCreateDevice( dv, &createInfo, nullptr, &suitableDevice.device );
                if ( res != VK_SUCCESS )
                {
                    LOGW( "VK Failed to create logical device with queues for physical device %s.", suitableDevice.name.c_str() );
                    continue;
                }

                vkGetDeviceQueue( suitableDevice.device, *queueIndices.graphics, 0, &suitableDevice.graphicsQueue );
                vkGetDeviceQueue( suitableDevice.device, *queueIndices.compute, 0, &suitableDevice.computeQueue );
                vkGetDeviceQueue( suitableDevice.device, *queueIndices.transfer, 0, &suitableDevice.transferQueue );
                vkGetDeviceQueue( suitableDevice.device, *queueIndices.sparse, 0, &suitableDevice.sparseQueue );
                vkGetDeviceQueue( suitableDevice.device, *queueIndices.present, 0, &suitableDevice.presentQueue );
                m_Devices.emplace_back( suitableDevice );
            }
        }
        if ( m_Devices.empty() )
        {
            LOGC("VK Did not find any suitable devices.");
            return false;
        }
        return true;
    }

    bool RenderManagerVK::createSurfaces()
    {
        bool res = false;
    #if FV_SDL
        res = SDL_Vulkan_CreateSurface( (SDL_Window*)m_CreateParams.windowHandles[0], m_Instance, &m_WindowSurface );
    #endif
        if ( !res )
        {
            LOGC( "VK Failed to create windows surface.");
            return false;
        }
        return true;
    }

    bool RenderManagerVK::isDeviceSuitable(VkPhysicalDevice device, String& deviceName, QueueFamilyIndicesVK& famIndices)
    {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        deviceName = deviceProperties.deviceName;

        // Query queue families
        uint32_t queueFamilyCount;
        Vector<VkQueueFamilyProperties> queueFamilies;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        queueFamilies.resize( queueFamilyCount );
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        for ( u32 i=0; i<queueFamilyCount; ++i )
        {
            auto& queueFam = queueFamilies[i];
            if ( queueFam.queueCount > 0 )
            {
                if ( (queueFam.queueFlags & VK_QUEUE_GRAPHICS_BIT) ) famIndices.graphics = i;
                if ( (queueFam.queueFlags & VK_QUEUE_COMPUTE_BIT) )  famIndices.compute = i;
                if ( (queueFam.queueFlags & VK_QUEUE_TRANSFER_BIT) ) famIndices.transfer = i;
                if ( (queueFam.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ) famIndices.sparse = i;
            }
            VkBool32 presentSupported;
            vkGetPhysicalDeviceSurfaceSupportKHR( device, i, m_WindowSurface, &presentSupported );
            if ( presentSupported ) famIndices.present = i;
        }

        return  deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && 
                deviceFeatures.geometryShader && famIndices.complete();
    }

    void RenderManagerVK::render(const class Camera* camera)
    {

    }

    VKAPI_ATTR VkBool32 VKAPI_CALL RenderManagerVK::debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, 
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        switch( messageSeverity )
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG( "VK Validation layer %s.", pCallbackData->pMessage );
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            break;
            LOGW( "VK Validation layer %s.", pCallbackData->pMessage );
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOGC( "VK Validation layer %s.", pCallbackData->pMessage );
            break;
        }
        return VK_FALSE;
    }

}
#endif
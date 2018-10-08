#include "RenderManagerVK.h"
#if FV_VULKAN
#include "GraphicsResourceVK.h"
#include "../Core/Algorithm.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
#include "../Core/ComponentManager.h"
#include "../Core/OSLayer.h"

namespace fv
{
    RenderManagerVK::~RenderManagerVK()
    {
        closeGraphics();
    }

    bool RenderManagerVK::initGraphics()
    {
        // Setup layers and extensions for instance and devices
        m_RequiredInstanceExtensions = { "VK_EXT_debug_report", "VK_EXT_debug_utils" };
        m_RequiredInstanceLayers = { "VK_LAYER_LUNARG_standard_validation" };
        m_RequiredPhysicalExtensions.clear();
        m_RequiredPhysicalLayers.clear();

        RenderSetup rs{};
        readRenderSetup( rs );
        if (!createWindows( rs ))
        {
            return false;
        }

        if (!checkRequiredExtensions(m_RequiredInstanceExtensions))
        {
            LOGC("VK Required instance extensions not available.");
            return false;
        }
        if (!checkRequiredLayers(m_RequiredInstanceLayers))
        {
            LOGC("VK Required instance layers not available.");
            return false;
        }

        if ( !createIntance("First App")) 
            return false;

        trySetupDebugCallback( false, false );

        // In case of main swap chain
        if ( m_MainWindow )
        {
            if ( !createSurface(m_MainWindow, m_MainSwapChain.surface) )
            {
                return false;
            }
        }

        // Create after surface in case of swap chain
        assert( m_MainSwapChain.surface );
        if ( !createDevices(m_MainSwapChain.surface) )
            return false;

        // Find device that can present 
        if ( m_MainWindow )
        {
            bool bSwapChainCreated = false;
            for ( auto& dv : m_Devices )
            {
                // Device for swap chain must have swap chain extension
                Vector<const char*> swapChainExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
                if ( !checkRequiredExtensions(swapChainExtension, dv.physical) )
                {
                    continue;
                }

                SwapChainParamsVK scParams {};
                scParams.device = &dv;
                scParams.surface = m_MainSwapChain.surface;
                scParams.width  = rs.mainWindowWidth;
                scParams.height = rs.mainWindowHeight;
                scParams.imageArrayLayerCount = 1; // 2 in case of stereo 
                scParams.imageCount = 3; // Try triple buffering

                if ( !createSwapChain( scParams, m_MainSwapChain ) )
                {
                    return false;
                }
                bSwapChainCreated = true;
                m_MainSwapChain.device = &dv;
                break;
            }

            if ( !bSwapChainCreated )
            {
                LOGC("VK Failed to create swap chain. No device found that supports it.");
                return false;
            }
        }

        
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
        for ( auto& imgView : m_MainSwapChain.imgViews )
        {
            if ( imgView && m_MainSwapChain.device->logical )
            {
                vkDestroyImageView( m_MainSwapChain.device->logical, imgView, nullptr );
            }
        }
        if ( m_MainSwapChain.device && m_MainSwapChain.device->logical && m_MainSwapChain.swapChain )
        {
            vkDestroySwapchainKHR( m_MainSwapChain.device->logical, m_MainSwapChain.swapChain, nullptr );
        }
        if ( m_Instance && m_MainSwapChain.surface )
        {
            vkDestroySurfaceKHR( m_Instance, m_MainSwapChain.surface, nullptr);
        }
        if ( m_Instance )
        {
            vkDestroyInstance(m_Instance, nullptr);
            m_Instance = nullptr;
        }
    }

    GraphicResource* RenderManagerVK::createGraphic()
    {
        // Recycle GraphicResources.
        GraphicResource* graphic = sc<GraphicResource*>(componentManager()->newComponent(GraphicResourceVK::type()));
        return graphic;
    }

    void RenderManagerVK::readRenderSetup( RenderSetup& rs )
    {
        // TODO read from config
        rs.createMainWindow = true;
        rs.mainWindowWidth = 1600;
        rs.mainWindowHeight = 900;
        rs.mainWindowName = "Main Window";
        rs.mainWindowFullscreen = false;
    }

    bool RenderManagerVK::createWindows(const RenderSetup& rs)
    {
        if ( rs.createMainWindow )
        {
            m_MainWindow = OSCreateWindow(rs.mainWindowName.c_str(), 100, 100, rs.mainWindowWidth, rs.mainWindowHeight, rs.mainWindowFullscreen, true, false);
            if (!m_MainWindow) return false;

        #if FV_SDL
            uint32_t extension_count;
            Vector<const char*> extensions;
            SDL_Vulkan_GetInstanceExtensions((SDL_Window*)m_MainWindow, &extension_count, nullptr);
            extensions.resize(extension_count);
            SDL_Vulkan_GetInstanceExtensions((SDL_Window*)m_MainWindow, &extension_count, extensions.data());
            for ( auto* c : extensions )
            {
                m_RequiredInstanceExtensions.emplace_back(c);
            }
        #endif
        }

        return true;
    }

    bool RenderManagerVK::createIntance(const String& appName)
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName.c_str();
        appInfo.applicationVersion = 1;
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = 1;
        appInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = (u32)m_RequiredInstanceExtensions.size();
        createInfo.ppEnabledExtensionNames = m_RequiredInstanceExtensions.data();
        createInfo.enabledLayerCount = (u32)m_RequiredInstanceLayers.size();
        createInfo.ppEnabledLayerNames = m_RequiredInstanceLayers.data();

        VkResult res = vkCreateInstance( &createInfo, nullptr, &m_Instance );
        if (res != VK_SUCCESS )
        {
            LOGC("VK Create instance failed.");
            return false;
        }

        return true;
    }

    bool RenderManagerVK::trySetupDebugCallback(bool includeVerbose, bool includeInfo)
    {
    #if !FV_DEBUG
        return false;
    #endif
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = 
            (includeVerbose?VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:0) |
            (includeInfo?VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:0) |
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

    bool RenderManagerVK::createDevices(VkSurfaceKHR mainSurface)
    {
        uint32_t deviceCount = 0;
        Vector<VkPhysicalDevice> physicalDevices;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        physicalDevices.resize( deviceCount );
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data());

        for ( auto& physical : physicalDevices )
        {
            DeviceVK dv {};
            dv.physical = physical;

            if ( !checkRequiredExtensions(m_RequiredPhysicalExtensions, physical) ||
                 !checkRequiredLayers(m_RequiredPhysicalLayers, physical) )
            {
                continue; // not suitable
            }

            storeDeviceProperties( dv );
            storeDeviceQueueFamilies( dv, mainSurface );

            if ( !(dv.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && dv.features.geometryShader) )
            {
                continue; // Not suitable
            }

            float priorities = 1.f;
            Set<u32> uniqueQueueIndices = { *dv.queueIndices.graphics, *dv.queueIndices.compute, *dv.queueIndices.transfer, *dv.queueIndices.sparse };
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
            createInfo.enabledLayerCount = (u32)m_RequiredInstanceLayers.size();
            createInfo.ppEnabledLayerNames = m_RequiredInstanceLayers.data();

            auto res = vkCreateDevice( dv.physical, &createInfo, nullptr, &dv.logical );
            if ( res != VK_SUCCESS )
            {
                LOGC( "VK Failed to create logical device for %s.", dv.properties.deviceName );
                return false;
            }

            if ( dv.queueIndices.graphics ) vkGetDeviceQueue(dv.logical, *dv.queueIndices.graphics, 0, &dv.graphicsQueue);
            if ( dv.queueIndices.compute )  vkGetDeviceQueue(dv.logical, *dv.queueIndices.compute, 0, &dv.computeQueue);
            if ( dv.queueIndices.transfer ) vkGetDeviceQueue(dv.logical, *dv.queueIndices.transfer, 0, &dv.transferQueue);
            if ( dv.queueIndices.sparse )   vkGetDeviceQueue(dv.logical, *dv.queueIndices.sparse, 0, &dv.sparseQueue);
            if ( dv.queueIndices.present )  vkGetDeviceQueue(dv.logical, *dv.queueIndices.present, 0, &dv.presentQueue);

            m_Devices.emplace_back( dv );
        }

        if ( m_Devices.empty() )
        {
            LOGC("VK Did not find any suitable devices.");
            return false;
        }
        return true;
    }

    bool RenderManagerVK::createSurface(const void* wHandle, VkSurfaceKHR& surface)
    {
    #if FV_SDL
        bool bResult = SDL_Vulkan_CreateSurface((SDL_Window*)wHandle, m_Instance, &surface);
        if ( !bResult )
        {
            LOGW("SDL_VK Failed to create vulkan window surface.");
            return false;
        }
        return true;
    #endif
        return false;
    }

    bool RenderManagerVK::createSwapChain(const SwapChainParamsVK& p, SwapChainVK& swapChain)
    {
        assert( p.device && p.surface && p.width != 0 && p.height != 0 );

        SwapChainInfoVK chainInfo;
        querySwapChainInfo(p.device->physical, p.surface, chainInfo);

        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR presentMode;
        VkExtent2D extend;
        if ( !chooseSwapChain( p.width, p.height, chainInfo, surfaceFormat, presentMode, extend ) )
        {
            return false;
        }

        u32 imageCount = Clamp<u32>(p.imageCount, (u32)chainInfo.capabilities.minImageCount, chainInfo.capabilities.maxImageCount);
        u32 imageArrayLayers = Clamp<u32>(p.imageArrayLayerCount, 1U, (u32)chainInfo.capabilities.maxImageArrayLayers);

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = p.surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extend;
        createInfo.imageArrayLayers = imageArrayLayers; // In case of 3d stereo rendering must be 2
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        assert( p.device->queueIndices.graphics.has_value() && p.device->queueIndices.present.has_value() );
        uint32_t queueFamilyIndices[] = { p.device->queueIndices.graphics.value(), p.device->queueIndices.present.value() };
        if ( queueFamilyIndices[0] != queueFamilyIndices[1] )
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = chainInfo.capabilities.currentTransform;   // Pre transform the image (eg flip horizontal)
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // If want to blend with other windows in system
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE; // Whether hidden pixels by other windows are obscured
        createInfo.oldSwapchain = VK_NULL_HANDLE; // TODO fix later

        if (vkCreateSwapchainKHR(p.device->logical, &createInfo, nullptr, &swapChain.swapChain) != VK_SUCCESS) 
        {
            LOGC( "VK Failed to create swap chain for device." );
            return false;
        }
    
        // Retreive swap chain images
        assert( swapChain.swapChain );
        uint32_t swapChainImgCount;
        vkGetSwapchainImagesKHR(p.device->logical, swapChain.swapChain, &swapChainImgCount, nullptr);
        swapChain.images.resize(swapChainImgCount);
        vkGetSwapchainImagesKHR(p.device->logical, swapChain.swapChain, &swapChainImgCount, swapChain.images.data());

        // Create image views on images in swap chain
        for ( auto& img : swapChain.images )
        {
            VkImageView imgView = createImageView( p.device->logical, img, surfaceFormat.format );
            if ( !imgView ) return false;
            swapChain.imgViews.emplace_back( imgView );
        }

        swapChain.surface = p.surface;
        swapChain.device = p.device;

        assert( swapChain.imgViews.size() == swapChain.images.size() );
        return true;
    }

    void RenderManagerVK::storeDeviceProperties(DeviceVK& device)
    {
        assert( device.physical );
        vkGetPhysicalDeviceProperties(device.physical, &device.properties);
        vkGetPhysicalDeviceFeatures(device.physical, &device.features);
    }

    void RenderManagerVK::storeDeviceQueueFamilies(DeviceVK& device, VkSurfaceKHR mainSurface)
    {
        assert( device.physical );
        uint32_t queueFamilyCount;
        Vector<VkQueueFamilyProperties> queueFamilies;
        vkGetPhysicalDeviceQueueFamilyProperties(device.physical, &queueFamilyCount, nullptr);
        queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device.physical, &queueFamilyCount, queueFamilies.data());
        for ( u32 i=0; i<queueFamilyCount; ++i )
        {
            auto& queueFam = queueFamilies[i];
            if ( queueFam.queueCount > 0 )
            {
                if ( (queueFam.queueFlags & VK_QUEUE_GRAPHICS_BIT) ) device.queueIndices.graphics = i;
                if ( (queueFam.queueFlags & VK_QUEUE_COMPUTE_BIT) )  device.queueIndices.compute = i;
                if ( (queueFam.queueFlags & VK_QUEUE_TRANSFER_BIT) ) device.queueIndices.transfer = i;
                if ( (queueFam.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ) device.queueIndices.sparse = i;
            }
            VkBool32 presentSupported = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device.physical, i, mainSurface, &presentSupported);
            if ( presentSupported ) device.queueIndices.present = i;
        }
    }

    bool RenderManagerVK::checkRequiredExtensions(const Vector<const char*>& requiredList, VkPhysicalDevice physicalDevice)
    {
        uint32_t extensionCount = 0;
        Vector<VkExtensionProperties> extensions;
        Vector<String> foundExtensions;
        if ( !physicalDevice ) vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        else vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
        extensions.resize(extensionCount);
        if ( !physicalDevice ) vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        else vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());
        for ( u32 i=0; i< extensionCount; ++i ) foundExtensions.emplace_back(extensions[i].extensionName);
        return validateNameList(foundExtensions, requiredList);
    }

    bool RenderManagerVK::checkRequiredLayers(const Vector<const char*>& requiredList, VkPhysicalDevice physicalDevice)
    {
    #if !FV_DEBUG
        return true;
    #endif

        uint32_t layerCount;
        Vector<VkLayerProperties> layers;
        Vector<String> foundLayers;
        if ( !physicalDevice ) vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        else vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, nullptr);
        layers.resize(layerCount);
        if ( !physicalDevice ) vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
        else vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, layers.data());
        for ( u32 i = 0; i < layerCount; i++ ) foundLayers.emplace_back(layers[i].layerName);
        return validateNameList(foundLayers, requiredList);
    }

    bool RenderManagerVK::validateNameList(const Vector<String>& found, const Vector<const char*>& required)
    {
        for ( auto* e : required )
        {
            if ( !Contains(found, e) )
            {
                LOGC("VK Cannot find requested %s.", e);
                return false;
            }
        }
        return true;
    }

    VkImageView RenderManagerVK::createImageView(VkDevice device, VkImage image, VkFormat format)
    {
        assert( device && image && format );

        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if ( vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS )
        {
            LOGC( "VK failed to create image view." );
            return nullptr;
        }
        return imageView;
    }

    void RenderManagerVK::querySwapChainInfo(VkPhysicalDevice device, VkSurfaceKHR surface, SwapChainInfoVK& info)
    {
        uint32_t formatCount;
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &info.capabilities);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if ( formatCount > 0 )
        {
            info.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, info.formats.data());
        }
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if ( presentModeCount > 0 )
        {
            info.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, info.presentModes.data());
        }
    }

    bool RenderManagerVK::chooseSwapChain(u32 width, u32 height, const SwapChainInfoVK& info, 
                                          VkSurfaceFormatKHR& format, VkPresentModeKHR& mode, VkExtent2D& extend)
    {
        bool found = false;
        if ( info.formats.size() > 0 )
        {
            if ( info.formats[0].format == VK_FORMAT_UNDEFINED )
            {
                format = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
                found = true;
            }
            else
            {
                for ( const auto& f : info.formats )
                {
                    if ( f.format == VK_FORMAT_B8G8R8A8_UNORM  && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
                    {
                        format = f;
                        found = true;
                        break;
                    }
                }
            }
        }
        if ( !found )
        {
            LOGC("VK Could not find suitable surface format for swap chain.");
            return false;
        }
        mode  = VK_PRESENT_MODE_FIFO_KHR;
        for ( const auto& presentMode : info.presentModes )
        {
            if ( presentMode == VK_PRESENT_MODE_MAILBOX_KHR )
            {
                mode = presentMode;
                break;
            }
            else if ( presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR )
            {
                mode = presentMode;
            }
        }
        if ( info.capabilities.currentExtent.width != UINT_MAX )
        {
            extend = info.capabilities.currentExtent;
        }
        else
        {
            extend = { width, height };
            extend.width  = Clamp(extend.width, info.capabilities.minImageExtent.width, info.capabilities.maxImageExtent.width);
            extend.height = Clamp(extend.height, info.capabilities.minImageExtent.height, info.capabilities.maxImageExtent.height);
        }
        return true;
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
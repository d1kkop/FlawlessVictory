#include "RenderManagerVK.h"
#if FV_VULKAN
#include "HelperVK.h"
#include "GraphicResourceVK.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
#include "../Core/OSLayer.h"
#include "../Core/JobManager.h"

namespace fv
{
    RenderManagerVK::RenderManagerVK():
        m_Graphics(32, true)
    {
    }

    RenderManagerVK::~RenderManagerVK()
    {
        m_Graphics.purge(); // Ensures that graphic child resources are deleted before device is destroyed.
        closeGraphics();
    }

    bool RenderManagerVK::initGraphics()
    {
        // Setup layers and extensions for instance and devices
    #if FV_DEBUG
        m_RequiredInstanceExtensions = { "VK_EXT_debug_report", "VK_EXT_debug_utils" };
        m_RequiredInstanceLayers = { "VK_LAYER_LUNARG_standard_validation" };
        m_RequiredPhysicalLayers = { "VK_LAYER_LUNARG_standard_validation" };
    #endif
        m_RequiredPhysicalExtensions = { };

        RenderConfig rs{};
        readRenderConfig( rs );

        if ( rs.createMainWindow )
        {
            m_MainWindow = OSCreateWindow(rs.mainWindowName.c_str(), 100, 100, rs.mainWindowWidth, rs.mainWindowHeight, rs.mainWindowFullscreen, true, false);
            if ( !m_MainWindow )
            {
                LOGC("Failed to create main window %s.", rs.mainWindowName.c_str());
                return false;
            }
            m_RequiredPhysicalExtensions.emplace_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
            HelperVK::queryRequiredWindowsExtensions( m_MainWindow, m_RequiredInstanceExtensions );
        }

        if (!HelperVK::checkRequiredExtensions(m_RequiredInstanceExtensions))
        {
            LOGC("VK Required instance extensions not available.");
            return false;
        }
        if (!HelperVK::checkRequiredLayers(m_RequiredInstanceLayers))
        {
            LOGC("VK Required instance layers not available.");
            return false;
        }

        if ( !HelperVK::createInstance("First App", m_RequiredInstanceExtensions, m_RequiredInstanceLayers, m_Instance) )
        {
            LOGC("VK Failed to create instance.");
            return false;
        }

    #if FV_DEBUG
        if ( !HelperVK::createDebugCallback( m_Instance, false, true, debugCallback, m_DebugCallback ) )
        {
            LOGC("VK Failed to create debug callback.");
            return false;
        }
    #endif

        // In case of main swap chain
        if ( m_MainWindow )
        {
            if ( !HelperVK::createSurface(m_Instance, m_MainWindow, m_MainSwapChain.surface) )
            {
                LOGC("VK Failed to create main window surface.");
                return false;
            }
        }

        // Note, surface can null in case there is no mainWindow.
        if ( !createDevices(m_MainSwapChain.surface) )
            return false;

        if ( m_MainWindow )
        {
            // Find device that can present 
            bool bSwapChainCreated = false;
            for ( auto& dv : m_Devices )
            {
                SwapChainParamsVK scParams {};
                scParams.device = &dv;
                scParams.surface = m_MainSwapChain.surface;
                scParams.width  = rs.mainWindowWidth;
                scParams.height = rs.mainWindowHeight;
                scParams.imageArrayLayerCount = 1; // 2 in case of stereo 
                scParams.imageCount = 3; // Try triple buffering

                if ( !HelperVK::createSwapChain(scParams, m_MainSwapChain) )
                {
                    return false;
                }
                bSwapChainCreated = true;
                m_MainSwapChain.device = &dv;
                break; // Only pick a single device for mainWindow
            }

            if ( !bSwapChainCreated )
            {
                LOGC("VK Failed to create swap chain. No device found that supports it.");
                return false;
            }
        }

        // Standard default shaders necessary to set up a pipeline
        createStandardShaders();

        // Create pipelines
        for ( auto& dv : m_Devices )
        {
            VkExtent2D vpSize = { rs.mainWindowWidth, rs.mainWindowHeight };
            if ( !HelperVK::createBasePipeline( dv.logical, nullptr, nullptr, nullptr, vpSize, dv.opaquePipelineLayout, dv.opaquePipeline ) )
            {
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
        for ( auto& dv : m_Devices )
        {
            if ( dv.logical )
            {
                if ( dv.opaquePipelineLayout ) vkDestroyPipelineLayout( dv.logical, dv.opaquePipelineLayout, nullptr );
                if ( dv.opaquePipeline) vkDestroyPipeline( dv.logical, dv.opaquePipeline, nullptr );
                vkDestroyDevice( dv.logical, nullptr );
            }
        }
        if ( m_Instance )
        {
            vkDestroyInstance(m_Instance, nullptr);
        }
    }

    GraphicResource* RenderManagerVK::createGraphic(GraphicType type, u32 deviceIdx)
    {
        GraphicResourceVK* gr = m_Graphics.newObject(); // Is thread safe
        RenderManager::setGraphicType( gr, type );
        assert( m_Devices[deviceIdx].logical );
        gr->init(type);
        gr->m_Device = m_Devices[deviceIdx].logical;
        return gr;
    }

    void RenderManagerVK::freeGraphic(GraphicResource* resource, bool async)
    {
        if (!resource) return;
        if ( !async || IsEngineClosing() )
        {
            resource->freeResource();
            m_Graphics.freeObject( sc<GraphicResourceVK*>(resource) );
        }
        else
        {
            jobManager()->addJob( [=]()
            {
                resource->freeResource();
                m_Graphics.freeObject(sc<GraphicResourceVK*>(resource));
            }, true /*auto free job*/);
        }
    }

    void RenderManagerVK::readRenderConfig(RenderConfig& rs)
    {
        // TODO read from config
        rs.createMainWindow = true;
        rs.mainWindowWidth = 1600;
        rs.mainWindowHeight = 900;
        rs.mainWindowName = "Main Window";
        rs.mainWindowFullscreen = false;
    }

    bool RenderManagerVK::createDevices(VkSurfaceKHR surface)
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

            if ( !HelperVK::checkRequiredExtensions(m_RequiredPhysicalExtensions, physical) ||
                 !HelperVK::checkRequiredLayers(m_RequiredPhysicalLayers, physical) )
            {
                continue; // not suitable
            }

            HelperVK::storeDevicePropertiesAndFeatures( dv );
            HelperVK::storeDeviceQueueFamilies( dv, surface );

            if ( !((dv.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
                   dv.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) &&
                   dv.features.geometryShader) )
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

            if ( !HelperVK::createDevice( m_Instance, dv.physical, queueCreateInfos, m_RequiredPhysicalExtensions, m_RequiredPhysicalLayers, dv.logical ) )
            {
                LOGC( "VK Failed to create logical devices." );
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

    bool RenderManagerVK::createStandardShaders()
    {
        return true;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL RenderManagerVK::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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
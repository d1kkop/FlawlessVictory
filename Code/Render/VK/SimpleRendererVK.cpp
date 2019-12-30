#include "SimpleRendererVK.h"
#if FV_VULKAN
#include "../../Core/Functions.h"
#include "../../Core/LogManager.h"
#include "../../Core/OSLayer.h"
#include "../../Core/Thread.h"
#include "../../Core/IncGLFW.h"
#include "InstanceVK.h"
#include "DeviceVK.h"
#include "SurfaceVK.h"
#include "SwapChainVK.h"

namespace fv
{
    SimpleRendererVK::SimpleRendererVK()
    = default;

    SimpleRendererVK::~SimpleRendererVK()
    {
        closeGraphics();
    }

    FV_BG bool SimpleRendererVK::initGraphics()
    {
        FV_CHECK_BG();

        if (!createWindow()) return false;
        if (!createInstance()) return false;
        if (!createSurface()) return false;
        if (!createDevice()) return false;
        if (!createSwapChain()) return false;

        LOG( "VK Initialized succesful." );
        return true;
    }

    void SimpleRendererVK::closeGraphics()
    {
        destroyWindow();
    }

    void SimpleRendererVK::render()
    {
    }

    bool SimpleRendererVK::createInstance()
    {
        Vector<const char*> requiredExtensions;
        Vector<const char*> requiredLayers;

    #if FV_DEBUG
        requiredExtensions ={ "VK_EXT_debug_report", "VK_EXT_debug_utils" };
        requiredLayers ={ "VK_LAYER_LUNARG_standard_validation" };
    #endif

    #if FV_GLFW
        // Need these extensions for surface.
        const char** ext;
        uint32_t count;
        ext = glfwGetRequiredInstanceExtensions(&count);
        for ( uint32_t t = 0; t < count; t++) requiredExtensions.emplace_back( ext[t] );
    #endif

        m_Instance = InstanceVK::create( "SimpleVKRenderer", requiredExtensions, requiredLayers );
        if ( m_Instance )
        {
        #if FV_DEBUG
            m_Instance->createDebugCallback( false, false, SimpleRendererVK::debugCallback );
        #endif
            return true;
        }

        return false;
    }

    bool SimpleRendererVK::createSurface()
    {
        m_Surface = SurfaceVK::create( m_Instance, m_Window );
        return m_Surface != nullptr;
    }

    bool SimpleRendererVK::createDevice()
    {
        Vector<const char*> requiredExtensions;
        Vector<const char*> requiredLayers;

    #if FV_DEBUG
        requiredLayers ={ "VK_LAYER_LUNARG_standard_validation" };
    #endif
        requiredExtensions ={ };

        m_Device = DeviceVK::create( m_Instance, requiredExtensions, requiredLayers, true, true, true, 2, true, true, true, m_Surface );

        return m_Device != nullptr;
    }

    bool SimpleRendererVK::createSwapChain()
    {
        u32 width;
        u32 height;
        if ( !OSGetWindowSurfaceSize( m_Window, width, height ))
        {
            LOGC( "VK Failed to obtain window surface size." );
            return false;
        }
        Set<u32> graphicsAndPresentQueueFamIndices;
        graphicsAndPresentQueueFamIndices.insert( m_Device->graphicsQueueFamily() );
        graphicsAndPresentQueueFamIndices.insert( m_Device->computeQueueFamily() );
        m_SwapChain = SwapChainVK::create( m_Device, m_Surface, width, height, 2, 1, graphicsAndPresentQueueFamIndices );
        return m_SwapChain != nullptr;
    }

    bool SimpleRendererVK::createWindow()
    {
        m_Window = OSCreateWindow( "VKWindow", 0, 0, 1600, 900, false );
        return m_Window.invalid() == false;
    }

    void SimpleRendererVK::destroyWindow()
    {
        if ( !m_Window.invalid() )
        {
            OSDestroyWindow( m_Window );
            m_Window.setZero();
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL SimpleRendererVK::debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                    void* pUserData )
    {
        switch ( messageSeverity )
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
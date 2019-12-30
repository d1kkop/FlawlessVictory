#include "SimpleRendererVK.h"
#if FV_VULKAN
#include "../../Core/Functions.h"
#include "../../Core/LogManager.h"
#include "../../Core/OSLayer.h"
#include "../../Core/Thread.h"
#include "../../Core/IncGLFW.h"
#include "InstanceVK.h"
#include "DeviceVK.h"

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
        if (!createDevice()) return false;

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

        m_Instance = InstanceVK::create( "SimpleVKRenderer", requiredExtensions, requiredLayers );
        if ( m_Instance )
        {
        #if FV_DEBUG
            m_Instance->createDebugCallback( false, false, SimpleRendererVK::debugCallback );
        #endif
            return m_Instance->createWindowSurface( m_Window );
        }
        return false;
    }

    bool SimpleRendererVK::createDevice()
    {
        Vector<const char*> requiredExtensions;
        Vector<const char*> requiredLayers;

    #if FV_DEBUG
        requiredLayers ={ "VK_LAYER_LUNARG_standard_validation" };
    #endif
        requiredExtensions ={ };

        m_Device = DeviceVK::create( m_Instance, requiredExtensions, requiredLayers, true, true, true, 2, true, true, true, true );

        return true;
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
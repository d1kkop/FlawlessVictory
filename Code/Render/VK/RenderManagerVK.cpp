#include "RenderManagerVK.h"
#if FV_VULKAN
#include "../../Core/Functions.h"
#include "../../Core/LogManager.h"
#include "../../Core/OSLayer.h"
#include "../../Core/Thread.h"

namespace fv
{
    RenderManagerVK::RenderManagerVK()
    {
    }

    RenderManagerVK::~RenderManagerVK()
    {
        closeGraphics();
    }

    bool RenderManagerVK::initGraphics()
    {
        FV_CHECK_BG();

        // Setup layers and extensions for instance and devices
    #if FV_DEBUG
        m_RequiredInstanceExtensions ={ "VK_EXT_debug_report", "VK_EXT_debug_utils" };
        m_RequiredInstanceLayers ={ "VK_LAYER_LUNARG_standard_validation" };
        m_RequiredPhysicalLayers ={ "VK_LAYER_LUNARG_standard_validation" };
    #endif
        m_RequiredPhysicalExtensions ={ };

        LOG( "VK Initialized succesful." );
        return true;
    }

    void RenderManagerVK::closeGraphics()
    {
    }

    void RenderManagerVK::render()
    {
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL RenderManagerVK::debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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
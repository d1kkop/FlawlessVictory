#include "SurfaceVK.h"
#include "InstanceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"
#include "../../Core/OSLayer.h"
#include "../../Core/IncGLFW.h"

namespace fv
{
    SurfaceVK::~SurfaceVK()
    {
    #if FV_GLFW
        if ( m_Surface )
        {
            vkDestroySurfaceKHR( m_Instance->vk(), m_Surface, NULL );
        }
    #endif
    }

    M<SurfaceVK> SurfaceVK::create( const M<InstanceVK>& instance, const OSHandle& window )
    {
        if ( window.invalid() )
        {
            LOGC( "VK Invalid window handle. Cannot create surface." );
            return {};
        }
    #if (FV_GLFW)
        GLFWwindow* glfwWindow = window.get<GLFWwindow*>();
        VkSurfaceKHR surface {};
        if ( !glfwWindow )
        {
            LOGC( "VK Invalid window handle" );
            return {};
        }
        if ( glfwVulkanSupported() == GLFW_FALSE )
        {
            LOGC( "VK No Vulkan support available for glfw surface." );
            return false;
        }
        VK_CALL( glfwCreateWindowSurface( instance->vk(), glfwWindow, NULL, &surface ) );
        if ( surface )
        {
            M<SurfaceVK> surfaceVk = std::make_shared<SurfaceVK>();
            surfaceVk->m_Surface  = surface;
            surfaceVk->m_Instance = instance;
            return surfaceVk;
        }
    #else
    #error no impl
    #endif
        return {};
    }
}
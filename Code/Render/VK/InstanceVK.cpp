#include "InstanceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"
#include "../../Core/OSLayer.h"
#include "../../Core/IncGLFW.h"

namespace fv
{
    InstanceVK::~InstanceVK()
    {
        if ( m_HasDebugAttached )
        {
            auto destroyDebugUtilsMesgenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( m_Instance, "vkDestroyDebugUtilsMessengerEXT" );
            if ( destroyDebugUtilsMesgenger )
            {
                destroyDebugUtilsMesgenger( m_Instance, m_DbgUtilMessenger, nullptr );
            }
        }
    #if FV_GLFW
        if ( m_Surface )
        {
            vkDestroySurfaceKHR( m_Instance, m_Surface, NULL );
        }
    #endif
        if ( m_Instance )
        {
            vkDestroyInstance( m_Instance, NULL );
        }
    }

    M<InstanceVK> InstanceVK::create( const String& appName,
                                      const Vector<const char*>& requiredExtensions,
                                      const Vector<const char*>& requiredLayers )
    {
        VkApplicationInfo appInfo ={};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName.c_str();
        appInfo.applicationVersion = 1;
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = 1;
        appInfo.apiVersion = VK_API_VERSION_1_1; // VK_API_VERSION_1_0

        VkInstanceCreateInfo createInfo ={};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = (u32)requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = (u32)requiredLayers.size();
        createInfo.ppEnabledLayerNames = requiredLayers.data();

        VkInstance instance;
        if ( vkCreateInstance( &createInfo, nullptr, &instance ) != VK_SUCCESS )
        {
            LOGW( "VK Create instance failed." );
            return {};
        }

        M<InstanceVK> inst = std::make_shared<InstanceVK>();
        inst->m_Instance = instance;
        return inst;
    }

    bool InstanceVK::createDebugCallback( bool includeVerbose, bool includeInfo, PFN_vkDebugUtilsMessengerCallbackEXT cb )
    {
        assert( m_Instance && !m_HasDebugAttached );
        if ( m_HasDebugAttached) return true;
        VkDebugUtilsMessengerCreateInfoEXT createInfo ={};
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
        createInfo.pfnUserCallback = cb;
        createInfo.pUserData = nullptr; // Optional
        auto createDebugUtilsMesgenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( m_Instance, "vkCreateDebugUtilsMessengerEXT" );
        if ( !createDebugUtilsMesgenger ||
             createDebugUtilsMesgenger( m_Instance, &createInfo, nullptr, &m_DbgUtilMessenger ) != VK_SUCCESS )
        {
            LOGW( "VK Setup debug callback failed." );
            return false;
        }
        m_HasDebugAttached = true;
        return true;
    }

    bool InstanceVK::createWindowSurface( const OSHandle& window )
    {
        if ( window.invalid() )
        {
            LOGW( "VK Invalid window handle." );
            return false;
        }
    #if (FV_INCLUDE_WINHDR || FV_GLFW)
        GLFWwindow* glfwWindow = window.get<GLFWwindow*>();
        if ( !glfwWindow )
        {
            LOGW( "VK Invalid window handle" );
            return false;
        }
    #endif
    #if (FV_INCLUDE_WINHDR && FV_GLFW)
        VkWin32SurfaceCreateInfoKHR createInfo ={};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd  = glfwGetWin32Window( glfwWindow );
        createInfo.hinstance = GetModuleHandle( nullptr );
        VK_CALL( vkCreateWin32SurfaceKHR( m_Instance, &createInfo, nullptr, &m_Surface ) );
    #elif FV_GLFW
        // TODO: always fails..
        VK_CALL( glfwCreateWindowSurface( m_Instance, glfwWindow, NULL, &m_Surface ) );
    #else
    #error no impl
    #endif
        return false;
    }

}
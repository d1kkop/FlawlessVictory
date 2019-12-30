#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class InstanceVK
    {
    public:
        InstanceVK() = default;
        ~InstanceVK();

    public:
        static M<InstanceVK> create( const String& appName,
                                     const Vector<const char*>& requiredExtensions,
                                     const Vector<const char*>& requiredLayers );

        VkInstance inst() const { return m_Instance; }

    #if FV_GLFW
        VkSurfaceKHR surface() const { return m_Surface; }
    #endif

        bool createDebugCallback( bool showVerbose, bool showInfo, PFN_vkDebugUtilsMessengerCallbackEXT callback );
        bool createWindowSurface( const struct OSHandle& window );

    private:
        VkInstance m_Instance {};
        VkDebugUtilsMessengerEXT m_DbgUtilMessenger {};
        bool m_HasDebugAttached = false;
    #if FV_GLFW
        VkSurfaceKHR m_Surface {};
    #endif
    };
}
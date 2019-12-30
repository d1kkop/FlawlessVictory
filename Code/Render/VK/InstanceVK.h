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

        VkInstance vk() const { return m_Instance; }
        bool createDebugCallback( bool showVerbose, bool showInfo, PFN_vkDebugUtilsMessengerCallbackEXT callback );

    private:
        VkInstance m_Instance {};
        VkDebugUtilsMessengerEXT m_DbgUtilMessenger {};
        bool m_HasDebugAttached = false;
    };
}
#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class SurfaceVK
    {
    public:
        SurfaceVK() = default;
        ~SurfaceVK();

    public:
        static M<SurfaceVK> create( const M<class InstanceVK>& instance, const struct OSHandle& window );

    #if FV_GLFW
        VkSurfaceKHR vk() const { return m_Surface; }
    #endif

    private:
    #if FV_GLFW
        VkSurfaceKHR m_Surface {};
    #endif
        M<InstanceVK> m_Instance;
    };
}
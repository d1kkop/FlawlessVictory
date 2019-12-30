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
        VkSurfaceKHR vk() const { return m_Surface; }

    private:
        VkSurfaceKHR m_Surface {};
        M<InstanceVK> m_Instance;
    };
}
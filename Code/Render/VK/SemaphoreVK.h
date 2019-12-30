#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class DeviceVK;

    class SemaphoreVK
    {
    public:
        SemaphoreVK() = default;
        ~SemaphoreVK();

    public:
        static M<SemaphoreVK> create( const M<DeviceVK>& device );
        VkSemaphore vk() const { return m_Semaphore; }
        const M<DeviceVK>& device() const { return m_Device; }


    private:
        VkSemaphore m_Semaphore {};
        M<DeviceVK> m_Device;
    };
}
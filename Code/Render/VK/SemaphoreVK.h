#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class DeviceVK;

    class SemaphoreVK
    {
    public:
        using VkType = VkSemaphore;

    public:
        SemaphoreVK() = default;
        ~SemaphoreVK();

    public:
        static M<SemaphoreVK> create( const M<DeviceVK>& device );
        const M<DeviceVK>& device() const { return m_Device; }

        VkSemaphore vk() const { return m_Semaphore; }
        const VkSemaphore* vkp() const { return &m_Semaphore; }

    private:
        VkSemaphore m_Semaphore {};
        M<DeviceVK> m_Device;
    };
}
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
        static M<SemaphoreVK> create( const M<DeviceVK>& device, u32 num=1 );

        VkSemaphore vk(u32 idx=0) const { return m_Semaphores[idx]; }
        const u32 num() const { return (u32)m_Semaphores.size(); }
        const VkSemaphore* getAll() const { return m_Semaphores.data(); }

        const M<DeviceVK>& device() const { return m_Device; }


    private:
        List<VkSemaphore> m_Semaphores {};
        M<DeviceVK> m_Device;
    };
}
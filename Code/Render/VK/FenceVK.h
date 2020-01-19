#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class DeviceVK;

    class FenceVK
    {
    public:
        using VkType = VkFence;

    public:
        FenceVK() = default;
        ~FenceVK();

    public:
        static M<FenceVK> create( const M<DeviceVK>& device, bool startSignaled=false );
        const M<DeviceVK>& device() const { return m_Device; }

        VkFence vk() const { return m_Fence; }
        const VkFence* vkp() const { return &m_Fence; }
        void reset();
        void wait();

    private:
        VkFence m_Fence;
        M<DeviceVK> m_Device;
    };
}
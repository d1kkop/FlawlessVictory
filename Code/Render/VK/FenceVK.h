#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class DeviceVK;

    class FenceVK
    {
    public:
        FenceVK() = default;
        ~FenceVK();

    public:
        static M<FenceVK> create( const M<DeviceVK>& device, bool startSignaled=false );
        VkFence vk() const { return m_Fence; }
        const M<DeviceVK>& device() const { return m_Device; }

        void reset();
        void wait();

        static void waitForMultiple( const List<M<FenceVK>>& fences );
        static void waitForMultiple( VkDevice device, const List<VkFence>& fences );

    private:
        VkFence m_Fence {};
        M<DeviceVK> m_Device;
    };
}
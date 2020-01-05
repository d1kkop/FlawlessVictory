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
        static M<FenceVK> create( const M<DeviceVK>& device, bool startSignaled=false, u32 num=1 );
        VkFence vk() const { return m_Fences[0]; }
        const M<DeviceVK>& device() const { return m_Device; }

        void reset( u32 idx=0 );
        void resetAll();
        void wait( u32 idx=0 );
        void waitAll();

        u32 num() const { return (u32)m_Fences.size(); }
        const VkFence* getAll() const { return m_Fences.data(); }

        static void waitForMultiple( VkDevice device, const List<VkFence>& fences );

    private:
        List<VkFence> m_Fences;
        M<DeviceVK> m_Device;
    };
}
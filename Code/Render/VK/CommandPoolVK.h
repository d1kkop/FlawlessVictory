#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class DeviceVK;

    class CommandPoolVK
    {
    public:
        CommandPoolVK() = default;
        ~CommandPoolVK();

    public:
        static M<CommandPoolVK> create( const M<DeviceVK>& device, u32 forQueueFamily );

        void trimCommandPool();
        void resetCommandPool();

        VkCommandPool vk() const { return m_CommandPool; }
        const M<DeviceVK>& device() const { return m_Device; }

    private:
        VkCommandPool m_CommandPool {};
        M<DeviceVK> m_Device;
    };
}
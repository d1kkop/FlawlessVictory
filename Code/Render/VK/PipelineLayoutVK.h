#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class DeviceVK;

    class PipelineLayoutVK
    {
    public:
        PipelineLayoutVK() = default;
        ~PipelineLayoutVK();

    public:
        static M<PipelineLayoutVK> create( const M<DeviceVK>& device );

        VkPipelineLayout vk() const { return m_PipelineLayout; }
        const M<DeviceVK>& device() const { return m_Device; }

    private:
        VkPipelineLayout m_PipelineLayout {};
        M<DeviceVK> m_Device;
    };
}
#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "IncVulkan.h"

namespace fv
{
    struct PipelineVK
    {
    public:
        PipelineVK() = default;
        void release(); // Do not put in destructor.
        struct DeviceVK& device() const { return *m_Device; }
        VkPipeline pipeline() const { return m_Pipeline; }
        VkPipelineLayout layout() const { return m_Layout; }

    // TODO private:
        struct DeviceVK* m_Device;
        VkPipeline m_Pipeline;
        VkPipelineLayout m_Layout;
    };
}
#endif
#pragma once
#include "../Core/Common.h"
#if FV_VULKAN

namespace fv
{
    struct PipelineVK
    {
    public:
        PipelineVK() = default;
        void release(); // Do not put in destructor.

        struct DeviceVK* device;
        VkPipeline pipeline;
        VkPipelineLayout layout;
    };
}
#endif
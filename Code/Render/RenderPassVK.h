#pragma once
#include "PipelineVK.h"

namespace fv
{
    struct RenderPassVK
    {
        RenderPassVK() = default;
        ~RenderPassVK();
        static RenderPassVK create(const PipelineVK& pipeline, VkFormat format, u32 numSamples, bool depth);
        struct DeviceVK& device() const { return m_Pipeline.device(); }
        const PipelineVK& pipeline() const { return m_Pipeline; }
        VkRenderPass renderPass() const { return m_RenderPass; }
        VkFormat format() const { return m_Format; }
        u32 samples() const { return m_Samples; }

    private:
        bool m_Valid = false;
        PipelineVK m_Pipeline;
        VkRenderPass m_RenderPass;
        VkFormat m_Format;
        u32 m_Samples;
    };
}
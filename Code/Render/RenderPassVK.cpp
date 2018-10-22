#include "RenderPassVK.h"
#include "DeviceVK.h"
#include "HelperVK.h"

namespace fv
{
    RenderPassVK::~RenderPassVK()
    {
        vkDestroyRenderPass( device().logical, m_RenderPass, nullptr );
    }

    RenderPassVK RenderPassVK::create(const PipelineVK& pipeline, VkFormat format, u32 numSamples, bool depth)
    {
        assert(!depth && "Not implemented yet.");
        VkRenderPass renderPass;
        if ( !HelperVK::createRenderPass( pipeline.device().logical, format, numSamples, renderPass ) )
        {
            return {};
        }
        RenderPassVK rp = {};
        rp.m_RenderPass = renderPass;
        rp.m_Pipeline = pipeline;
        rp.m_Format  = format;
        rp.m_Samples = numSamples;
        return std::move( rp );
    }

}
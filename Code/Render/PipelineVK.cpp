#include "PipelineVK.h"
#if FV_VULKAN
#include "DeviceVK.h"
#include "HelperVK.h"
#include "../Core/JobManager.h"

namespace fv
{
    void PipelineVK::release()
    {
        if (!m_Device ||!m_Device->logical) return;
        vkDestroyPipeline( m_Device->logical, m_Pipeline, nullptr );
        vkDestroyPipelineLayout( m_Device->logical, m_Layout, nullptr );
    }

    PipelineVK PipelineVK::create(DeviceVK& device, u32 pipelineHash,
                                  const PipelineFormatVK& format,
                                  const VkViewport& vp,
                                  const Vector<VkVertexInputBindingDescription>& vertexBindings, 
                                  const Vector<VkVertexInputAttributeDescription>& vertexAttribs)
    {
        assert(device.logical && format.mdata.vertShader.resources[0] && format.mdata.fragShader.resources[0] && /*geomShader* && */ format.renderPass);
        VkPipeline pipeline;
        VkPipelineLayout layout;
        if ( !HelperVK::createPipeline(device.logical, format, vp, vertexBindings, vertexAttribs, pipeline, layout))
        {
            return {};
        }
        PipelineVK p;
        p.m_Valid = true;
        p.m_Device = &device;
        p.m_Pipeline = pipeline;
        p.m_Layout = layout;
        p.m_Hash = pipelineHash;
        p.m_Format = format;
        return std::move(p);
    }

    void PipelineVK::bind(VkCommandBuffer cb)
    {
        assert(cb && m_Pipeline);
        vkCmdBindPipeline( cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline );
    }

}
#endif
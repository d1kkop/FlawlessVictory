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
        m_Device->deleteCommandBuffers( m_BindBuffers );
        vkDestroyPipeline( m_Device->logical, m_Pipeline, nullptr );
        vkDestroyPipelineLayout( m_Device->logical, m_Layout, nullptr );
    }

    PipelineVK PipelineVK::create(DeviceVK& device, VkShaderModule vertShader, VkShaderModule fragShader, VkShaderModule geomShader, VkRenderPass renderPass, 
                                  const VkViewport& vp, u32 vertexSize,
                                  const Vector<VkVertexInputBindingDescription>& vertexBindings, 
                                  const Vector<VkVertexInputAttributeDescription>& vertexAttribs)
    {
        assert(device.logical && vertShader && fragShader && /*geomShader* && */ renderPass);
        VkPipeline pipeline;
        VkPipelineLayout layout;
        if ( !HelperVK::createPipeline(device.logical, vertShader, fragShader, geomShader, renderPass, vp, 
                                       vertexBindings, vertexAttribs, vertexSize, pipeline, layout ) )
        {
            return {};
        }
        PipelineVK p;
        p.m_Valid = true;
        p.m_Device = &device;
        p.m_Pipeline = pipeline;
        p.m_Layout = layout;
        return std::move(p);
    }

    void PipelineVK::recordCommandBuffers()
    {
        m_Device->recordCommandBuffers(m_BindBuffers, [&](VkCommandBuffer cb, VkFramebuffer fb)
        {
            recordBindBuffers(cb);
        });
    }

    void PipelineVK::addBindPipelineToQueue(struct FrameObject& fo, u32 queueIdx)
    {
        u32 numThreads = jobManager()->numThreads();
        fo.addCmdBufferToQueue( m_BindBuffers[numThreads*fo.idx() + queueIdx], queueIdx );
    }

    void PipelineVK::recordBindBuffers(VkCommandBuffer cb)
    {
        m_Device->recordCommandBuffers(m_BindBuffers, [this](VkCommandBuffer cb, VkFramebuffer fb)
        {
            vkCmdBindPipeline( cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline );
        });
    }

}
#endif
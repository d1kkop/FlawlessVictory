#include "RenderPassVK.h"
#if FV_VULKAN
#include "DeviceVK.h"
#include "HelperVK.h"
#include "../Core/JobManager.h"

namespace fv
{
    void RenderPassVK::release()
    {
        if (!m_Device || !m_Device->logical) return;
        m_Device->deleteCommandBuffers( m_BeginRenderPassBuffers );
        m_Device->deleteCommandBuffers( m_EndRenderPassBuffers );
        vkDestroyRenderPass( device().logical, m_RenderPass, nullptr );
    }

    RenderPassVK RenderPassVK::create(DeviceVK& device, VkFormat format, u32 numSamples, bool depth)
    {
        assert(device.logical && !depth && "Not implemented yet.");
        VkRenderPass renderPass;
        if ( !HelperVK::createRenderPass( device.logical, format, numSamples, renderPass ) )
        {
            return {};
        }
        RenderPassVK rp = {};
        rp.m_Valid  = true;
        rp.m_Device = &device;
        rp.m_RenderPass = renderPass;
        rp.m_Format  = format;
        rp.m_Samples = numSamples;

        return std::move( rp );
    }

    void RenderPassVK::recordCommandBuffers(const VkOffset2D& offset, const VkExtent2D& extend,
                                            const Vector<VkClearValue>& clearValues)
    {
        m_Device->recordCommandBuffers(m_BeginRenderPassBuffers, [&](VkCommandBuffer cb, VkFramebuffer fb)
        {
            recordBeginRenderPass(cb, fb, offset, extend, clearValues);
        });
        m_Device->recordCommandBuffers(m_EndRenderPassBuffers, [&](VkCommandBuffer cb, VkFramebuffer fb)
        {
            recordEndRenderPass(cb);
        });
    }

    void RenderPassVK::addBeginPassToQueue(FrameObject& fo, u32 queueIdx)
    {
        u32 numThreads = jobManager()->numThreads();
        fo.addCmdBufferToQueue(m_BeginRenderPassBuffers[fo.idx()*numThreads + queueIdx], queueIdx);
    }

    void RenderPassVK::addEndPassToQueue(FrameObject& fo, u32 queueIdx)
    {
        u32 numThreads = jobManager()->numThreads();
        fo.addCmdBufferToQueue(m_EndRenderPassBuffers[fo.idx()*numThreads + queueIdx], queueIdx);
    }

    void RenderPassVK::recordBeginRenderPass(VkCommandBuffer cb, VkFramebuffer frameBuffer,
                                             const VkOffset2D& offset, const VkExtent2D& extent,
                                             const Vector<VkClearValue>& clearValues)
    {
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass  = m_RenderPass;
        renderPassInfo.framebuffer = frameBuffer;
        renderPassInfo.renderArea.offset = offset;
        renderPassInfo.renderArea.extent = extent;

        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass( cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    }

    void RenderPassVK::recordEndRenderPass(VkCommandBuffer cb)
    {
        vkCmdEndRenderPass(cb);
    }

}
#endif
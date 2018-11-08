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
        vkDestroyRenderPass( device().logical, m_RenderPass, nullptr );
        m_RenderPass = nullptr;
    }

    RenderPassVK RenderPassVK::create(DeviceVK& device, u32 numAttachments, u32 numSubpasses, u32 numDependencies,
                                      const Function<void (u32 idx, VkAttachmentDescription& atd, VkAttachmentReference& atr)>& attachCb,
                                      const Function<void (u32 idx, VkSubpassDescription& subpass, const Vector<VkAttachmentReference>&)>& subpassCb,
                                      const Function<void (u32 idx, VkSubpassDependency& dependency)>& dependencyCb)
    {
        Vector<VkAttachmentDescription> attachments(numAttachments);
        Vector<VkAttachmentReference> attRefs(numAttachments);

        for ( u32 i=0; i< numAttachments; ++i )
        {
            VkAttachmentDescription atd = {};
            atd.samples = (VkSampleCountFlagBits)1;
            atd.loadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            atd.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            atd.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            atd.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            atd.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            atd.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference atr = {};
            atr.attachment = i;
            atr.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            attachCb (i, atd, atr);

            attachments[i] = atd;
            attRefs[i] = atr;
        }

        Vector<VkSubpassDescription> subpasses(numSubpasses);
        for ( u32 i=0; i<numSubpasses; ++i )
        {
            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassCb(i, subpass, attRefs);
            subpasses[i] = subpass;
        }

        Vector<VkSubpassDependency> dependencies(numDependencies);
        for ( u32 i=0; i<numSubpasses; ++i )
        {
            VkSubpassDependency depen = {};
            dependencyCb( i, depen );
            dependencies[i] = depen;
        }

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = (u32)attachments.size();
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = (u32)subpasses.size();
        renderPassInfo.pSubpasses = subpasses.data();

        VkRenderPass renderPass;
        if ( vkCreateRenderPass(device.logical, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS )
        {
            LOGC("VK Failed to create render pass.");
            return {};
        }

        RenderPassVK rp = {};
        rp.m_Valid  = true;
        rp.m_Device = &device;
        rp.m_RenderPass = renderPass;

        return std::move(rp);
    }

    void RenderPassVK::begin(VkCommandBuffer cb, VkFramebuffer fb,
                             const VkOffset2D& offset, const VkExtent2D& extent,
                             const Vector<VkClearValue>& clearValues)
    {
        assert(cb && fb);
        HelperVK::startRenderPass( cb, m_RenderPass, fb, { offset, extent }, clearValues.data() );
    }

    void RenderPassVK::end( VkCommandBuffer cb )
    {
        assert(cb);
        HelperVK::stopRenderPass( cb );
    }
}
#endif
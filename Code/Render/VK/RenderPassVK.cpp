#include "RenderPassVK.h"
#include "DeviceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    RenderPassVK::~RenderPassVK()
    {
        if ( m_RenderPass )
        {
            vkDestroyRenderPass( m_Device->logical(), m_RenderPass, NULL );
        }
    }

    M<RenderPassVK> RenderPassVK::create( const M<DeviceVK>& device, VkFormat attachmentFormat, u32 numSamplesPerPixelPowerOf2,
                                          AttachmentLoadOp loadOp, AttachmentSaveOp saveOp,
                                          AttachmentLoadOp stencilLoadOp, AttachmentSaveOp stencilSaveOp )

    {
        VkAttachmentDescription colorAttachment ={};
        colorAttachment.format  = attachmentFormat;
        colorAttachment.samples = (VkSampleCountFlagBits) numSamplesPerPixelPowerOf2;
        colorAttachment.loadOp  = (VkAttachmentLoadOp) loadOp;
        colorAttachment.storeOp = (VkAttachmentStoreOp) saveOp;
        colorAttachment.stencilLoadOp  = (VkAttachmentLoadOp) stencilLoadOp;
        colorAttachment.stencilStoreOp = (VkAttachmentStoreOp) stencilSaveOp;

        VkAttachmentReference colorAttachmentRef ={};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass ={};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo ={};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        VkRenderPass renderPass;
        VK_CALL( vkCreateRenderPass( device->logical(), &renderPassInfo, NULL, &renderPass ) );

        auto renderPassVk = std::make_shared<RenderPassVK>();
        renderPassVk->m_RenderPass = renderPass;
        renderPassVk->m_Device = device;
        return renderPassVk;
    }

}
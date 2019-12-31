#include "FrameBufferVK.h"
#include "DeviceVK.h"
#include "HelperVK.h"
#include "RenderPassVK.h"
#include "ImageViewVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    FrameBufferVK::~FrameBufferVK()
    {
        if ( m_FrameBuffer )
        {
            vkDestroyFramebuffer( m_RenderPass->device()->logical(), m_FrameBuffer, NULL );
        }
    }

    M<FrameBufferVK> FrameBufferVK::create( const M<RenderPassVK>& renderPass, const List<M<ImageViewVK>>& imgAttachments, const VkExtent2D& extent, u32 numLayers )
    {
        List<VkImageView> attachments;
        HelperVK::toVkList( imgAttachments, attachments );
        VkFramebuffer frameBuffer;
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass->vk();
        framebufferInfo.attachmentCount = (u32)attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width  = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = numLayers;
        if ( VK_SUCCESS != vkCreateFramebuffer( renderPass->device()->logical(), &framebufferInfo, NULL, &frameBuffer ) )
        {
            LOGC( "VK failed to create framebuffer." );
            return {};
        }
        auto frameBufferVk = std::make_shared<FrameBufferVK>();
        frameBufferVk->m_FrameBuffer = frameBuffer;
        frameBufferVk->m_RenderPass  = renderPass;
        frameBufferVk->m_ImageViews  = imgAttachments;
        return frameBufferVk;
    }

}
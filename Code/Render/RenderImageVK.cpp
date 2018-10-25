#include "RenderManager.h" // For config
#if FV_VULKAN
#include "RenderImageVK.h"
#include "DeviceVK.h"
#include "HelperVK.h"
#include "MemoryHelperVK.h"

namespace fv
{
    bool RenderImageVK::initialize(DeviceVK& device, const struct RenderConfig& rc, VkImage swapChainImage, VkRenderPass renderPass)
    {
        m_Device = &device;

        if ( !swapChainImage && !createImage(rc) )
            return false;

      return createImageView(rc, swapChainImage) && createFrameBuffer(renderPass);
    }

    void RenderImageVK::release()
    {
        if (!m_Device ||!m_Device->logical) return;
        if ( m_Image.allocation ) MemoryHelperVK::freeImage( m_Image ); /* If has swap chain, images are from not set up. */
        vkDestroyImageView( m_Device->logical, m_ImageView, nullptr );
        vkDestroyFramebuffer( m_Device->logical, m_FrameBuffer, nullptr );
    }

    bool RenderImageVK::createImage(const struct RenderConfig& rc)
    {
        if ( !MemoryHelperVK::createImage(*m_Device, m_Device->extent.width, m_Device->extent.height, m_Device->format, 1,
                                          rc.numLayers, rc.numSamples, false, m_Device->queueIndices.graphics.value(),
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, m_Image) )
        {
            return false;
        }
        return true;
    }

    bool RenderImageVK::createImageView(const struct RenderConfig& rc, VkImage swapChainImage)
    {
        VkImage chosenImage = m_Image.image;
        if ( swapChainImage )
        {
            assert( !chosenImage );
            chosenImage = swapChainImage;
        }
        if ( !HelperVK::createImageView(m_Device->logical, chosenImage, m_Device->format, rc.numLayers, m_ImageView) )
        {
            return false;
        }
        return true;
    }

    bool RenderImageVK::createFrameBuffer(VkRenderPass renderPass)
    {
        assert(renderPass);
        Vector<VkImageView> attachments = { m_ImageView };
        if ( !HelperVK::createFramebuffer(m_Device->logical, m_Device->extent, renderPass, attachments, m_FrameBuffer) )
        {
            return false;
        }
        return true;
    }

}
#endif
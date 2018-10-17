#include "RenderManager.h" // For config
#if FV_VULKAN
#include "RenderImageVK.h"
#include "DeviceVK.h"
#include "HelperVK.h"
#include "MemoryHelperVK.h"

namespace fv
{
    void RenderImageVK::release()
    {
        if (!device ||!device->logical) return;
        if ( imageAlloc.allocation ) MemoryHelperVK::freeImage( imageAlloc ); /* If has swap chain, images are from not set up. */
        vkDestroyImageView( device->logical, imgView, nullptr );
        vkDestroyFramebuffer( device->logical, frameBuffer, nullptr );
        vkFreeCommandBuffers( device->logical, device->commandPool, (u32)commandBuffers.size(), commandBuffers.data() );
    }

    bool RenderImageVK::createImage(const struct RenderConfig& rc)
    {
        if ( !MemoryHelperVK::createImage(*device, device->extent.width, device->extent.height, device->format, 1,
                                          rc.numLayers, rc.numSamples, false, device->queueIndices.graphics.value(),
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, imageAlloc) )
        {
            return false;
        }
        return true;
    }

    bool RenderImageVK::createImageView(const struct RenderConfig& rc, VkImage swapChainImage)
    {
        VkImage chosenImage = imageAlloc.image;
        if ( swapChainImage )
            chosenImage = swapChainImage;
        if ( !HelperVK::createImageView(device->logical, chosenImage, device->format, rc.numLayers, imgView) )
        {
            return false;
        }
        return true;
    }

    bool RenderImageVK::createFrameBuffer(VkRenderPass renderPass)
    {
        assert(renderPass);
        Vector<VkImageView> attachments = { imgView };
        if ( !HelperVK::createFramebuffer(device->logical, device->extent, renderPass, attachments, frameBuffer) )
        {
            return false;
        }
        return true;
    }

}
#endif
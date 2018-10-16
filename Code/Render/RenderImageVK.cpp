#include "PCH.h"
#if FV_VULKAN
#include "RenderManager.h" // For config
#include "RenderImageVK.h"
#include "DeviceVK.h"
#include "HelperVK.h"

namespace fv
{
    void RenderImageVK::release()
    {
        if (!device ||!device->logical) return;
        vkDestroyImage(device->logical, image, nullptr);
        vkFreeMemory(device->logical, imageMemory, nullptr);
        vkDestroyImageView( device->logical, imgView, nullptr );
        vkDestroyFramebuffer( device->logical, frameBuffer, nullptr );
        vkFreeCommandBuffers( device->logical, device->commandPool, (u32)commandBuffers.size(), commandBuffers.data() );
    }

    bool RenderImageVK::createImage(const struct RenderConfig& rc)
    {
        // Set format and extent as no swap chain is present.
        if ( !HelperVK::createImage(device->logical, device->memProperties,
                                    device->extent, 1, device->format, rc.numSamples, rc.numLayers, false, 
                                    device->queueIndices.graphics.value(), image, imageMemory ))
        {
            return false;
        }
        return true;
    }

    bool RenderImageVK::createImageView(const struct RenderConfig& rc, VkImage swapChainImage)
    {
        VkImage chosenImage = image;
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
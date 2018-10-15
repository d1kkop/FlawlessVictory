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
        vkDestroyImage(device->logical, image, nullptr);
        vkFreeMemory(device->logical, imageMemory, nullptr);
        vkDestroyImageView( device->logical, imgView, nullptr );
        vkDestroyFramebuffer( device->logical, frameBuffer, nullptr );
        vkFreeCommandBuffers( device->logical, device->commandPool, (u32)commandBuffers.size(), commandBuffers.data() );
    }

    bool RenderImageVK::createImages(const struct RenderConfig& rc)
    {
        if ( device->swapChain ) return true; // Images from swap chain.
        // Set format and extent as no swap chain is present.
        if ( !HelperVK::createImage(device->logical, device->memProperties,
                                    device->extent, 1, device->format, rc.numSamples, rc.numLayers, false, 
                                    device->queueIndices.graphics.value(), image, imageMemory ))
        {
            return false;
        }
        return true;
    }

    bool RenderImageVK::createImageViews(const struct RenderConfig& rc)
    {
        if ( !HelperVK::createImageView(device->logical, image, device->format, rc.numLayers, imgView) )
        {
            return false;
        }
        return false;
    }

    bool RenderImageVK::createFrameBuffers(VkRenderPass renderPass)
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
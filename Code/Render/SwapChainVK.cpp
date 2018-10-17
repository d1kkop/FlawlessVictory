#include "SwapChainVK.h"
#if FV_VULKAN
#include "DeviceVK.h"
#include "../Core/LogManager.h"

namespace fv
{
    void SwapChainVK::release()
    {
        if (!device ||!device->logical) return;
        // No need to delete images
        vkDestroySwapchainKHR( device->logical, swapChain, nullptr );
        vkDestroySwapchainKHR( device->logical, oldSwapChain, nullptr );
        vkDestroySurfaceKHR( device->instance, surface, nullptr );
    }

    bool SwapChainVK::createImages(u32 numLayers)
    {
        assert(device && device->logical && numLayers!=0);
        uint32_t swapChainImgCount;
        if ( vkGetSwapchainImagesKHR(device->logical, swapChain, &swapChainImgCount, nullptr) != VK_SUCCESS )
        {
            LOGW("VK Failed to obtain swap chain image count.");
            return false;
        }
        images.resize(swapChainImgCount);
        if ( vkGetSwapchainImagesKHR(device->logical, swapChain, &swapChainImgCount, images.data()) != VK_SUCCESS )
        {
            LOGW("VK Failed to create swap chain images.");
            return false;
        }
        return true;
    }

}
#endif
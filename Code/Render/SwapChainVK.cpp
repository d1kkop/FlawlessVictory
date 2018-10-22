#include "SwapChainVK.h"
#if FV_VULKAN
#include "DeviceVK.h"
#include "HelperVK.h"
#include "../Core/LogManager.h"
#include "../Core/Thread.h"

namespace fv
{
    SwapChainVK* SwapChainVK::create(struct DeviceVK& device, VkSurfaceKHR surface,
                                     u32 width, u32 height, u32 numImages, u32 numLayers,
                                     const Optional<u32>& graphicsQueueIdx, const Optional<u32>& presentQueueIdx)
    {
        assert( width > 0 && height > 0 && numImages > 0 && numLayers > 0 );
        VkSurfaceFormatKHR chosenFormat;
        VkPresentModeKHR chosenPresentMode;
        VkExtent2D surfaceExtend;
        VkSwapchainKHR swapChain;
        if ( !HelperVK::createSwapChain(device.logical, device.physical, surface,
                                        width, height, numImages, numLayers, 
                                        graphicsQueueIdx, presentQueueIdx,
                                        chosenFormat, chosenPresentMode, surfaceExtend, swapChain ))
        {
            return nullptr;
        }
        SwapChainVK* sc = new SwapChainVK{};
        sc->m_Device = &device;
        sc->m_Extent = surfaceExtend;
        sc->m_Surface = surface;
        sc->m_PresentMode = chosenPresentMode;
        sc->m_SurfaceFormat = chosenFormat;
        sc->m_SwapChain = swapChain;
        if ( !sc->getImages() )
        {
            vkDestroySwapchainKHR(device.logical, swapChain, nullptr);
            delete sc;
            return nullptr;
        }
        return sc;
    }

    SwapChainVK::~SwapChainVK()
    {
        assert( m_Device && m_Device->logical );
        // No need to delete images
        vkDestroySwapchainKHR(m_Device->logical, m_SwapChain, nullptr);
        vkDestroySwapchainKHR(m_Device->logical, m_OldSwapChain, nullptr);
        vkDestroySurfaceKHR(m_Device->instance, m_Surface, nullptr);
    }

    bool SwapChainVK::getImages()
    {
        assert(m_Device && m_Device->logical);
        uint32_t swapChainImgCount;
        if ( vkGetSwapchainImagesKHR(m_Device->logical, m_SwapChain, &swapChainImgCount, nullptr) != VK_SUCCESS )
        {
            LOGW("VK Failed to obtain swap chain image count.");
            return false;
        }
        m_Images.resize(swapChainImgCount);
        if ( vkGetSwapchainImagesKHR(m_Device->logical, m_SwapChain, &swapChainImgCount, m_Images.data()) != VK_SUCCESS )
        {
            LOGW("VK Failed to create swap chain images.");
            return false;
        }
        return true;
    }

    void SwapChainVK::acquireNextImage(u32& imageIndex, VkSemaphore semaphore, VkFence fence)
    {
        FV_CHECK_MO();
        assert( m_Device && m_Device->logical && m_SwapChain && semaphore );
        vkAcquireNextImageKHR(m_Device->logical, m_SwapChain, (u64)-1, semaphore, fence, (uint32_t*)&imageIndex);
    }

}
#endif
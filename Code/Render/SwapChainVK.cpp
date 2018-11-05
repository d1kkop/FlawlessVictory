#include "SwapChainVK.h"
#if FV_VULKAN
#include "DeviceVK.h"
#include "HelperVK.h"
#include "../Core/LogManager.h"
#include "../Core/Thread.h"

namespace fv
{
    SwapChainVK* SwapChainVK::create(struct DeviceVK& device, VkSurfaceKHR surface, u32 numFramesBehind,
                                     u32 width, u32 height, u32 numImages, u32 numLayers,
                                     const Optional<u32>& graphicsQueueIdx, const Optional<u32>& presentQueueIdx)
    {
        assert( width > 0 && height > 0 && numImages > 0 && numLayers > 0 && numFramesBehind > 0 );
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        Vector<VkSemaphore> imageAvailableSemaphores;
        for ( u32 i=0; i<numFramesBehind; ++i )
        {
            VkSemaphore imageAvailableSemaphore;
            if ( vkCreateSemaphore(device.logical, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS )
            {
                LOGC("VK Cannot create swap chain image available semaphore.");
                return nullptr;
            }
            imageAvailableSemaphores.emplace_back( imageAvailableSemaphore );
        }
        VkSurfaceFormatKHR chosenFormat;
        VkPresentModeKHR chosenPresentMode;
        VkExtent2D surfaceExtend;
        VkSwapchainKHR swapChain;
        if ( !HelperVK::createSwapChain(device.logical, device.physical, surface,
                                        width, height, numImages, numLayers, 
                                        graphicsQueueIdx, presentQueueIdx,
                                        chosenFormat, chosenPresentMode, surfaceExtend, swapChain ))
        {
            for ( auto s : imageAvailableSemaphores ) vkDestroySemaphore( device.logical, s, nullptr );
            return nullptr;
        }
        SwapChainVK* sc = new SwapChainVK{};
        sc->m_Device = &device;
        sc->m_Extent = surfaceExtend;
        sc->m_Surface = surface;
        sc->m_PresentMode = chosenPresentMode;
        sc->m_SurfaceFormat = chosenFormat;
        sc->m_SwapChain = swapChain;
        sc->m_ImageAvailableSemaphores = std::move(imageAvailableSemaphores);
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
        for (auto& s : m_ImageAvailableSemaphores) vkDestroySemaphore( m_Device->logical, s, nullptr );
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

    VkSemaphore SwapChainVK::acquireNextImage(u32 frameIndex, u32& renderImageIndex)
    {
        FV_CHECK_MO();
        assert( m_Device && m_Device->logical && m_SwapChain );
        FV_VKCALL( vkAcquireNextImageKHR(m_Device->logical, m_SwapChain, (u64)-1, m_ImageAvailableSemaphores[frameIndex], nullptr, (uint32_t*)&renderImageIndex) );
        return m_ImageAvailableSemaphores[frameIndex];
    }

}
#endif
#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "IncVulkan.h"

namespace fv
{
    struct SwapChainVK
    {
    public:
        SwapChainVK() = default;
        ~SwapChainVK();
        static SwapChainVK* create(struct DeviceVK& device, VkSurfaceKHR surface,
                                   u32 width, u32 height, u32 numImages, u32 numLayers,
                                   const Optional<u32>& graphicsQueueIdx, const Optional<u32>& presentQueueIdx);
        FV_MO VkSemaphore acquireNextImage(u32& imageIndex, VkFence fence);


        struct DeviceVK& device() { return *m_Device; }
        VkSurfaceKHR surface() const { return m_Surface; }
        VkExtent2D extent() const { return m_Extent; }
        VkPresentModeKHR presentMode() const { return m_PresentMode; }
        VkSurfaceFormatKHR surfaceFormat() const { return m_SurfaceFormat; }
        VkSwapchainKHR swapChain() const { return m_SwapChain; }
        VkSwapchainKHR oldSwapChain() const { return m_OldSwapChain; }
        const Vector<VkImage>& images() const { return m_Images; }

    private:
        bool getImages();

        struct DeviceVK* m_Device;
        VkSurfaceKHR m_Surface;
        VkExtent2D m_Extent;
        VkPresentModeKHR m_PresentMode;
        VkSurfaceFormatKHR m_SurfaceFormat;
        VkSwapchainKHR m_SwapChain;
        VkSwapchainKHR m_OldSwapChain;
        Vector<VkImage> m_Images;
        VkSemaphore m_ImageAvailableSemaphore;
    };
}
#endif
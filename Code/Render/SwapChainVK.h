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
        static SwapChainVK* create(struct DeviceVK& device, VkSurfaceKHR surface, u32 numFramesBehind,
                                   u32 width, u32 height, u32 numImages, u32 numLayers,
                                   const Optional<u32>& graphicsQueueIdx, const Optional<u32>& presentQueueIdx,
                                   VkSwapchainKHR oldSwapChain);

        // Returns false if swap chain must be recreated.
        bool acquireNextImage(u32 frameIndex, u32& renderImageOut, VkSemaphore& waitSemaphore);

        // Returns false if swap chain must be created.
        bool present(const VkSemaphore* waitSemaphores, u32 numSemaphores);

        struct DeviceVK& device() { return *m_Device; }
        VkSurfaceKHR surface() const { return m_Surface; }
        VkExtent2D extent() const { return m_Extent; }
        VkPresentModeKHR presentMode() const { return m_PresentMode; }
        VkSurfaceFormatKHR surfaceFormat() const { return m_SurfaceFormat; }
        VkSwapchainKHR swapChain() const { return m_SwapChain; }
        const Vector<VkImage>& images() const { return m_Images; }

    private:
        bool getImages();

        struct DeviceVK* m_Device;
        VkSurfaceKHR m_Surface;
        VkExtent2D m_Extent;
        VkPresentModeKHR m_PresentMode;
        VkSurfaceFormatKHR m_SurfaceFormat;
        VkSwapchainKHR m_SwapChain;
        Vector<VkImage> m_Images;
        Vector<VkSemaphore> m_ImageAvailableSemaphores;
        u32 m_RenderImageIndex;
    };
}
#endif
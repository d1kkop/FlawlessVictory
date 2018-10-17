#pragma once
#include "../Core/Common.h"
#if FV_VULKAN

namespace fv
{
    struct SwapChainVK
    {
    public:
        SwapChainVK() = default;
        void release(); // Do not put in destructor.
        bool createImages(u32 numLayers);

        struct DeviceVK* device;
        VkSurfaceKHR surface;
        VkExtent2D extent;
        VkPresentModeKHR presentMode;
        VkSurfaceFormatKHR surfaceFormat;
        VkSwapchainKHR swapChain;
        VkSwapchainKHR oldSwapChain;
        Vector<VkImage> images;
    };
}
#endif
#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "PCH.h"

namespace fv
{
    struct RenderImageVK
    {
    public:
        RenderImageVK() = default;
        void release(); // Do not put in destructor.
        bool createImages(const struct RenderConfig& rc);
        bool createImageViews(const struct RenderConfig& rc);
        bool createFrameBuffers(VkRenderPass renderPass);

        struct DeviceVK* device;
        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imgView;
        VkFramebuffer frameBuffer;
        Vector<VkCommandBuffer> commandBuffers;
    };
}
#endif
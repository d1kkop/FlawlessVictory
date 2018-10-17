#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "MemoryHelperVK.h"

namespace fv
{
    struct RenderImageVK
    {
    public:
        RenderImageVK() = default;
        void release(); // Do not put in destructor.
        bool createImage(const struct RenderConfig& rc);
        bool createImageView(const struct RenderConfig& rc, VkImage swapChainImg);
        bool createFrameBuffer(VkRenderPass renderPass);

        struct DeviceVK* device;
        ImageVK imageAlloc;
        VkImageView imgView;
        VkFramebuffer frameBuffer;
        Vector<VkCommandBuffer> commandBuffers;
    };
}
#endif
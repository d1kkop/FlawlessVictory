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
        bool initialize(struct DeviceVK& device, const struct RenderConfig& rc, VkImage swapChainImage, VkRenderPass renderPass);
        void release(); // Do not put in destructor.
        VkFramebuffer frameBuffer() { return m_FrameBuffer; }

    private:
        bool createImage(const struct RenderConfig& rc);
        bool createImageView(const struct RenderConfig& rc, VkImage swapChainImg);
        bool createFrameBuffer(VkRenderPass renderPass);

        struct DeviceVK* m_Device;
        ImageVK m_Image;
        VkImageView m_ImageView;
        VkFramebuffer m_FrameBuffer;
    };
}
#endif
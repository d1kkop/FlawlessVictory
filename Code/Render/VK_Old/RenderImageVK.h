#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "IncVulkan.h"

namespace fv
{
    struct ImageVK;

    struct RenderImageVK
    {
    public:
        RenderImageVK() = default;
        bool initialize(struct DeviceVK& device, VkRenderPass renderPass, const Function<bool (u32 idx, VkImage&, VkFormat&, VkImageAspectFlags& flags, u32& layers)>& createImageCb);
        void release(); // Do not put in destructor.

        DeviceVK* device() const { return m_Device; }
        VkFramebuffer frameBuffer() { return m_FrameBuffer; }

    private:
        struct DeviceVK* m_Device;
        Vector<VkImageView> m_ImageViews;
        VkFramebuffer m_FrameBuffer;
    };
}
#endif
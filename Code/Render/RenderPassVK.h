#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "IncVulkan.h"

namespace fv
{
    struct DeviceVK;

    struct RenderPassVK
    {
        RenderPassVK() = default;
        void release();
        static RenderPassVK create(DeviceVK& device, VkFormat format, u32 numSamples, bool depth);

        bool valid() const { return m_Valid; }
        struct DeviceVK& device() const { return *m_Device; }
        VkRenderPass renderPass() const { return m_RenderPass; }
        VkFormat format() const { return m_Format; }
        u32 samples() const { return m_Samples; }

        void begin(VkCommandBuffer cb, VkFramebuffer fb,
                   const VkOffset2D& offset, const VkExtent2D& extent,
                   const Vector<VkClearValue>& clearValues);
        void end(VkCommandBuffer cb);

    private:
        bool m_Valid = false;
        DeviceVK* m_Device;
        VkRenderPass m_RenderPass;
        VkFormat m_Format;
        u32 m_Samples;
    };
}
#endif
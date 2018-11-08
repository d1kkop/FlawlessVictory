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
        static RenderPassVK create(DeviceVK& device, u32 numAttachments, u32 numSubpasses, u32 numDependencies,
                                   const Function<void (u32 idx, VkAttachmentDescription& atd, VkAttachmentReference& atr)>& attachCb,
                                   const Function<void (u32 idx, VkSubpassDescription& subpass, const Vector<VkAttachmentReference>&)>& subpassCb,
                                   const Function<void (u32 idx, VkSubpassDependency& dependency)>& dependencyCb);

        bool valid() const { return m_Valid; }
        struct DeviceVK& device() const { return *m_Device; }
        VkRenderPass renderPass() const { return m_RenderPass; }

        void begin(VkCommandBuffer cb, VkFramebuffer fb,
                   const VkOffset2D& offset, const VkExtent2D& extent,
                   const Vector<VkClearValue>& clearValues);
        void end(VkCommandBuffer cb);

    private:
        bool m_Valid = false;
        DeviceVK* m_Device;
        VkRenderPass m_RenderPass;
    };
}
#endif
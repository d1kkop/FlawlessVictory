#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    enum class AttachmentLoadOp
    {
        Load,
        Clear,
        DontCare
    };

    enum class AttachmentSaveOp
    {
        Save,
        DontCare
    };

    class DeviceVK;

    class RenderPassVK
    {
    public:
        RenderPassVK() = default;
        ~RenderPassVK();

    public:
        static M<RenderPassVK> create( const M<DeviceVK>& device, VkFormat attachmentFormat, u32 numSamplesPerPixelPowerOf2=1,
                                       AttachmentLoadOp loadOp = AttachmentLoadOp::Clear, AttachmentSaveOp saveOp = AttachmentSaveOp::Save,
                                       AttachmentLoadOp stencilLoadOp = AttachmentLoadOp::DontCare, AttachmentSaveOp stencilSaveOp = AttachmentSaveOp::DontCare );
        /* TODO add other create functions for more sophisticated render pass set ups. */

        //void beginCmd( VkCommandBu

        VkRenderPass vk() const { return m_RenderPass; }
        const M<DeviceVK>& device() const { return m_Device; }

    private:
        VkRenderPass m_RenderPass {};
        M<DeviceVK> m_Device;
    };
}
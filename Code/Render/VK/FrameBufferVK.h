#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class RenderPassVK;
    class ImageViewVK;

    class FrameBufferVK
    {
    public:
        FrameBufferVK() = default;
        ~FrameBufferVK();

    public:
        static M<FrameBufferVK> create( const M<RenderPassVK>& renderPass, const List<M<ImageViewVK>>& imgAttachments, const VkExtent2D& extent, u32 numLayers=1 );

        VkFramebuffer vk() const { return m_FrameBuffer; }
        u32 numImageViews() const { return (u32)m_ImageViews.size(); }

        const M<RenderPassVK>& renderPass() const { return m_RenderPass; }
        const List<M<ImageViewVK>>& imageViews() const { return m_ImageViews; }
        const M<ImageViewVK>& imageView( u32 idx ) const { return m_ImageViews[idx]; }

    private:
        VkFramebuffer m_FrameBuffer {};
        M<RenderPassVK> m_RenderPass;
        List<M<ImageViewVK>> m_ImageViews;
    };
}
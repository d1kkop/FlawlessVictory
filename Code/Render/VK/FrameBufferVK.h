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


    private:
        VkFramebuffer m_FrameBuffer {};
        M<RenderPassVK> m_RenderPass;
        List<M<ImageViewVK>> m_ImageViews;
    };
}
#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    enum class CommandBufferUsage
    {
        OneTime,
        EntirelyInRenderPass,
        CanResubmit
    };

    class CommandPoolVK;
    class RenderPassVK;
    class FrameBufferVK;
    class PipelineVK;
    class BufferVK;
    class ImageVK;

    class CommandBufferVK
    {
    public:
        using VkType = VkCommandBuffer;

    public:
        CommandBufferVK() = default;
        ~CommandBufferVK();

    public:
        static M<CommandBufferVK> allocate( const M<CommandPoolVK>& fromPool, bool fromSecondary=false );

        VkCommandBuffer vk() const { return m_CommandBuffer; }
        const VkCommandBuffer* vkp() const { return &m_CommandBuffer; }
        void resetBufferState();

        void beginRecord( CommandBufferUsage usage );
        void beginRenderPass( const M<RenderPassVK>& renderPass, const M<FrameBufferVK>& frameBuffer,
                              const Vec4& clearColor ={ .5f, .5f, 1.f, 1 }, const VkOffset2D& offset ={ 0, 0 }, const VkExtent2D& extent ={ (uint32_t)-1, (uint32_t)-1 } );

        // Pipeline, vertices and descriptors
        void bindPipeline( const M<PipelineVK>& pipeline );
        void bindVertices( const M<BufferVK>& vertexBuffer );

        // Drawing
        void draw( u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0 );
        void drawIndexed( u32 indexCount, u32 instanceCount = 1, u32 firstIndex = 0, u32 vertexOffset = 0, u32 firstInstance = 0 );
        void drawIndirect( const M<BufferVK>& buffer, u64 offset, u32 drawCount, u32 stride );
        void drawIndexedIndirect( const M<BufferVK>& buffer, u64 offset, u32 drawSize, u32 stride );

        // Fixed function
        void setViewport( u32 viewportIdx, u32 viewportCount, const VkViewport* pViewports );
        void setScissor( u32 scissorIdx, u32 scissorCount, const VkRect2D* scissorRects );
        void setLineWidth( float lineWidth );
        void setDepthBias( float constantFactor, float clamp, float slopeFactor );

        void endRenderPass();

        void clearColorImage( const M<ImageVK>& image, VkImageLayout imageLayout, const Vec4& clearColor = { .5f, .5f, 1.f, 1 } ); // Transfer VK_IMAGE_LAYOUT_DST_OPTIMAL, GENERAL or SHARED_PRESENT_KHR
        void clearDepthStencilImage( const M<ImageVK>& image, VkImageLayout imageLayout, float depthValue, u32 stencilValue ); // VK_IMAGE_LAYOUT_TRANDER_DST_OPTIMAL or GENERAL
        void fillBuffer( const M<BufferVK>& dstBuffer, u64 dstOffset, u64 size, u32 data );
        void updateBuffer( const M<BufferVK>& dstBuffer, u64 offset, u64 size, const void* data );
        void copyBuffer( const M<BufferVK>& srcBuffer, const M<BufferVK>& dstBuffer, u32 regionCount, u64 size, u64 srcOffset = 0, u64 dstOffset = 0 );
        void copyImage( const M<ImageVK>& srcImage, const M<ImageVK>& dstImage, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout, const VkImageCopy* regions, u32 numRegions );
        void copyImageColorSingle( const M<ImageVK>& srcImage, const M<ImageVK>& dstImage, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout );
        void blitImage( const M<ImageVK>& srcImage, M<ImageVK>& dstImage, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout, const VkImageBlit* regions, u32 numRegions, VkFilter filter = VK_FILTER_LINEAR );
        void blitImageSingle( const M<ImageVK>& srcImage, M<ImageVK>& dstImage, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout, VkFilter filter = VK_FILTER_LINEAR );

        // Stencil & Blend
        void setBlendConstants( float blendConstants[4] );
        void setDepthBounds( float minDepthBounds, float maxDepthBounds );
        void setStencilCompareMask( VkStencilFaceFlags faceMask, u32 compareMask );
        void setStencilWriteMask( VkStencilFaceFlags faceMask, u32 writeMask );
        void setStencilReference( VkStencilFaceFlags faceMask, u32 reference );

        // Compute
        void dispatch( u32 groupCountX, u32 groupCountY, u32 groupCountZ );
        void dispatchIndirect( const M<BufferVK>& buffer, u64 offset );
        void dispatchBase( u32 baseGroupX, u32 baseGroupY, u32 baseGroupZ, u32 groupCountX, u32 groupCountY, u32 groupCountZ );

        void endRecord();

        const M<CommandPoolVK>& commandPool() const { return m_CommandPool; }

    private:
        VkCommandBuffer m_CommandBuffer;
        M<CommandPoolVK> m_CommandPool;
    };
}
#include "CommandBufferVK.h"
#include "CommandPoolVK.h"
#include "DeviceVK.h"
#include "RenderPassVK.h"
#include "PipelineVK.h"
#include "BufferVK.h"
#include "ImageVK.h"
#include "FrameBufferVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    CommandBufferVK::~CommandBufferVK()
    {
        if ( m_CommandBuffer)
        {
            vkFreeCommandBuffers( m_CommandPool->device()->logical(), m_CommandPool->vk(), 1, &m_CommandBuffer );
        }
    }

    M<CommandBufferVK> CommandBufferVK::allocate( const M<CommandPoolVK>& pool, bool fromSecondary )
    {
        VkCommandBufferAllocateInfo allocInfo ={};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = pool->vk();
        allocInfo.level = fromSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        VkCommandBuffer buffer;
        VK_CALL( vkAllocateCommandBuffers( pool->device()->logical(), &allocInfo, &buffer ) );
        auto commBuffersVK = std::make_shared<CommandBufferVK>();
        commBuffersVK->m_CommandPool = pool;
        commBuffersVK->m_CommandBuffer = buffer;
        return commBuffersVK;
    }

    void CommandBufferVK::resetBufferState()
    {
        VK_CALL_VOID( vkResetCommandBuffer( m_CommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT ) );
    }

    void CommandBufferVK::beginRecord( CommandBufferUsage usage )
    {
        u32 us = 0;
        switch (usage)
        {
        case CommandBufferUsage::OneTime:
            us = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            break;
        case CommandBufferUsage::EntirelyInRenderPass:
            us = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            break;
        case CommandBufferUsage::CanResubmit:
            us = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            break;
        }
        VkCommandBufferBeginInfo beginInfo ={};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = us;
        VK_CALL_VOID( vkBeginCommandBuffer( m_CommandBuffer, &beginInfo ) );
    }

    void CommandBufferVK::beginRenderPass( const M<RenderPassVK>& renderPass, const M<FrameBufferVK>& frameBuffer,
                                           const Vec4& cc, const VkOffset2D& offset, const VkExtent2D& extent )
    {
        VkRenderPassBeginInfo renderPassInfo ={};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass->vk();
        renderPassInfo.framebuffer = frameBuffer->vk();
        renderPassInfo.renderArea.offset = offset;
        if ( extent.height==-1 && extent.width==-1 )
        {
            renderPassInfo.renderArea.extent = frameBuffer->extent();
        }
        else
        {
            renderPassInfo.renderArea.extent = extent;
        }
        VkClearValue clearColor ={ cc.x, cc.y, cc.z, cc.w };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass( m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    }

    void CommandBufferVK::bindPipeline( const M<PipelineVK>& pipeline )
    {
        vkCmdBindPipeline( m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk() );
    }

    void CommandBufferVK::bindVertices( const M<BufferVK>& vertexBuffer )
    {
        VkBuffer buffers[] ={ vertexBuffer->vk() };
        VkDeviceSize offsets[] ={ 0 };
        vkCmdBindVertexBuffers( m_CommandBuffer, 0, 1, buffers, offsets );
    }

    void CommandBufferVK::draw( u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance )
    {
        vkCmdDraw( m_CommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance );
    }

    void CommandBufferVK::drawIndexed( u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance )
    {
        vkCmdDrawIndexed( m_CommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance );
    }

    void CommandBufferVK::drawIndirect( const M<BufferVK>& buffer, u64 offset, u32 drawCount, u32 stride )
    {
        vkCmdDrawIndirect( m_CommandBuffer, buffer->vk(), offset, drawCount, stride );
    }

    void CommandBufferVK::drawIndexedIndirect( const M<BufferVK>& buffer, u64 offset, u32 drawSize, u32 stride )
    {
        vkCmdDrawIndexedIndirect( m_CommandBuffer, buffer->vk(), offset, drawSize, stride );
    }

    void CommandBufferVK::setViewport( u32 viewportIdx, u32 viewportCount, const VkViewport* pViewports )
    {
        vkCmdSetViewport( m_CommandBuffer, viewportIdx, viewportCount, pViewports );
    }

    void CommandBufferVK::setScissor( u32 scissorIdx, u32 scissorCount, const VkRect2D* scissorRects )
    {
        vkCmdSetScissor( m_CommandBuffer, scissorIdx, scissorCount, scissorRects );
    }

    void CommandBufferVK::setLineWidth( float lineWidth )
    {
        vkCmdSetLineWidth( m_CommandBuffer, lineWidth );
    }

    void CommandBufferVK::setDepthBias( float constantFactor, float clamp, float slopeFactor )
    {
        vkCmdSetDepthBias( m_CommandBuffer, constantFactor, clamp, slopeFactor );
    }

    void CommandBufferVK::endRenderPass()
    {
        vkCmdEndRenderPass( m_CommandBuffer );
    }

    void CommandBufferVK::clearColorImage( const M<ImageVK>& image, VkImageLayout imageLayout, const Vec4& clearColor )
    {
        VkClearColorValue clearVal {};
        memcpy( clearVal.float32, &clearColor.x, sizeof(float)*4 );
        vkCmdClearColorImage( m_CommandBuffer, image->vk(), imageLayout, &clearVal, 0, NULL );
    }

    void CommandBufferVK::clearDepthStencilImage( const M<ImageVK>& image, VkImageLayout imageLayout, float depthValue, u32 stencilValue )
    {
        VkClearDepthStencilValue clearVal;
        clearVal.depth = depthValue;
        clearVal.stencil = stencilValue;
        vkCmdClearDepthStencilImage( m_CommandBuffer, image->vk(), imageLayout, &clearVal, 0, NULL );
    }

    void CommandBufferVK::fillBuffer( const M<BufferVK>& dstBuffer, u64 dstOffset, u64 size, u32 data )
    {
        vkCmdFillBuffer( m_CommandBuffer, dstBuffer->vk(), dstOffset, size, data );
    }

    void CommandBufferVK::updateBuffer( const M<BufferVK>& dstBuffer, u64 offset, u64 size, const void* data )
    {
        vkCmdUpdateBuffer( m_CommandBuffer, dstBuffer->vk(), offset, size, data );
    }

    void CommandBufferVK::copyBuffer( const M<BufferVK>& srcBuffer, const M<BufferVK>& dstBuffer, u32 regionCount, u64 size, u64 srcOffset, u64 dstOffset )
    {
        VkBufferCopy bufferCopy;
        bufferCopy.dstOffset = dstOffset;
        bufferCopy.srcOffset = srcOffset;
        bufferCopy.size = size;
        vkCmdCopyBuffer( m_CommandBuffer, srcBuffer->vk(), dstBuffer->vk(), regionCount, &bufferCopy );
    }

    void CommandBufferVK::copyImage( const M<ImageVK>& srcImage, const M<ImageVK>& dstImage, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout, const VkImageCopy* regions, u32 numRegions )
    {
        vkCmdCopyImage( m_CommandBuffer, srcImage->vk(), srcImageLayout, dstImage->vk(), dstImageLayout, numRegions, regions );
    }

    void CommandBufferVK::copyImageColorSingle( const M<ImageVK>& srcImage, const M<ImageVK>& dstImage, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout )
    {
        assert( srcImage->width() == dstImage->width() && srcImage->height() == dstImage->height() );
        VkImageCopy imageCopy = {};
        imageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopy.dstSubresource.layerCount = 1;
        imageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopy.dstSubresource.layerCount = 1;
        imageCopy.extent.width  = srcImage->width();
        imageCopy.extent.height = srcImage->height();
        imageCopy.extent.depth  = 0;
        copyImage( srcImage, dstImage, srcImageLayout, dstImageLayout, &imageCopy, 1 );
    }

    void CommandBufferVK::blitImage( const M<ImageVK>& srcImage, M<ImageVK>& dstImage, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout, const VkImageBlit* regions, u32 numRegions, VkFilter filter )
    {
        vkCmdBlitImage( m_CommandBuffer, srcImage->vk(), srcImageLayout, dstImage->vk(), dstImageLayout, 1, regions, filter );
    }

    void CommandBufferVK::blitImageSingle( const M<ImageVK>& srcImage, M<ImageVK>& dstImage, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout, VkFilter filter )
    {
        assert( srcImage->width() == dstImage->width() && srcImage->height() == dstImage->height() );
        VkImageBlit blit {};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.layerCount = 1;
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.layerCount = 1;
        blitImage( srcImage, dstImage, srcImageLayout, dstImageLayout, &blit, 1, filter );
    }

    void CommandBufferVK::setBlendConstants( float blendConstants[4] )
    {
        vkCmdSetBlendConstants( m_CommandBuffer, blendConstants );
    }

    void CommandBufferVK::setDepthBounds( float minDepthBounds, float maxDepthBounds )
    {
        vkCmdSetDepthBounds( m_CommandBuffer, minDepthBounds, maxDepthBounds );
    }

    void CommandBufferVK::setStencilCompareMask( VkStencilFaceFlags faceMask, u32 compareMask )
    {
        vkCmdSetStencilCompareMask( m_CommandBuffer, faceMask, compareMask );
    }

    void CommandBufferVK::setStencilWriteMask( VkStencilFaceFlags faceMask, u32 writeMask )
    {
        vkCmdSetStencilWriteMask( m_CommandBuffer, faceMask, writeMask );
    }

    void CommandBufferVK::setStencilReference( VkStencilFaceFlags faceMask, u32 reference )
    {
        vkCmdSetStencilReference( m_CommandBuffer, faceMask, reference );
    }

    void CommandBufferVK::dispatch( u32 groupCountX, u32 groupCountY, u32 groupCountZ )
    {
        vkCmdDispatch( m_CommandBuffer, groupCountX, groupCountY, groupCountZ );
    }

    void CommandBufferVK::dispatchIndirect( const M<BufferVK>& buffer, u64 offset )
    {
        vkCmdDispatchIndirect( m_CommandBuffer, buffer->vk(), offset );
    }

    void CommandBufferVK::dispatchBase( u32 baseGroupX, u32 baseGroupY, u32 baseGroupZ, u32 groupCountX, u32 groupCountY, u32 groupCountZ )
    {
        vkCmdDispatchBase( m_CommandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ );
    }

    void CommandBufferVK::endRecord()
    {
        VK_CALL_VOID( vkEndCommandBuffer( m_CommandBuffer ) );
    }

}
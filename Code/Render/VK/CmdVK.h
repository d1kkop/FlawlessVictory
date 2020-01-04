#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class RenderPassVK;
    class FrameBufferVK;
    class PipelineVK;
    class SemaphoreVK;
    class SwapChainVK;

    class CmdVK
    {
    public:
        static void beginRenderPass( VkCommandBuffer cmdBuffer, const M<RenderPassVK>& renderPass, const M<FrameBufferVK>& frameBuffer,
                                     const Vec4& clearColor = { 0, 0, 0, 1 }, const VkOffset2D& offset = { 0, 0 }, const VkExtent2D& extent = { (uint32_t)-1, (uint32_t)-1 } );
        static void endRenderPass( VkCommandBuffer cmdBuffer );

        static void bindPipeline( VkCommandBuffer cmdBuffer, const M<PipelineVK>& pipeline );
        static void draw( VkCommandBuffer cmdBuffer, u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0 );

        static VkResult queuePresent( const M<SwapChainVK>& swapChain, u32 imageIndex, const List<M<SemaphoreVK>>& waitSemaphores );
    };
}
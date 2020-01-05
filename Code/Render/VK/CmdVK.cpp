#include "CmdVK.h"
#include "DeviceVK.h"
#include "BufferVK.h"
#include "FenceVK.h"
#include "RenderPassVK.h"
#include "FrameBufferVK.h"
#include "PipelineVK.h"
#include "SemaphoreVK.h"
#include "SwapChainVK.h"
#include "CommandBuffersVK.h"
#include "HelperVK.h"

namespace fv
{

    void CmdVK::beginRenderPass( VkCommandBuffer cmdBuffer, const M<RenderPassVK>& renderPass, const M<FrameBufferVK>& frameBuffer,
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
        VkClearValue clearColor = { cc.x, cc.y, cc.z, cc.w };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass( cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    }

    void CmdVK::endRenderPass( VkCommandBuffer cmdBuffer )
    {
        vkCmdEndRenderPass( cmdBuffer );
    }

    void CmdVK::bindPipeline( VkCommandBuffer cmdBuffer, const M<PipelineVK>& pipeline )
    {
        vkCmdBindPipeline( cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk() );
    }

    void CmdVK::bindVertices( VkCommandBuffer cmdBuffer, const M<BufferVK>& vertexBuffer )
    {
        VkBuffer buffers [] = { vertexBuffer->vk() };
        VkDeviceSize offsets [] = { 0 };
        vkCmdBindVertexBuffers( cmdBuffer, 0, 1, buffers, offsets );
    }

    void CmdVK::draw( VkCommandBuffer cmdBuffer, u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance )
    {
        vkCmdDraw( cmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance );
    }

    VkResult CmdVK::queueSubmit( VkQueue queue,
                                 const M<CommandBuffersVK>& commandBuffer,
                                 const M<SemaphoreVK>& waitSemaphores, 
                                 const M<SemaphoreVK>& signalSemaphores, 
                                 VkPipelineStageFlags waitDstStageMask,
                                 const M<FenceVK>& fenceToSignal )

    {
        VkSubmitInfo submitInfo ={};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = waitSemaphores->num();
        submitInfo.pWaitSemaphores    = waitSemaphores->getAll();
        submitInfo.pWaitDstStageMask  = &waitDstStageMask;
        submitInfo.commandBufferCount = commandBuffer->num();
        submitInfo.pCommandBuffers    = commandBuffer->getAll();
        submitInfo.signalSemaphoreCount = signalSemaphores->num();
        submitInfo.pSignalSemaphores    = signalSemaphores->getAll();
        return vkQueueSubmit( queue, 1, &submitInfo, fenceToSignal ? fenceToSignal->vk() : VK_NULL_HANDLE );
    }

    VkResult CmdVK::queuePresent( const M<SwapChainVK>& swapChain, u32 imageIndex, const M<SemaphoreVK>& waitSemaphores )
    {
        VkQueue presentQueue = swapChain->device()->presentQueue();
        VkPresentInfoKHR presentInfo ={};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = waitSemaphores->num();
        presentInfo.pWaitSemaphores = waitSemaphores->getAll();
        presentInfo.swapchainCount  = 1;
        VkSwapchainKHR swapChains [] = { swapChain->vk() };
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = (uint32_t*) &imageIndex;
        presentInfo.pResults = NULL;
        return vkQueuePresentKHR( presentQueue, &presentInfo );
    }

}
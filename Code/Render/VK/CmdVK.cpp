#include "CmdVK.h"
#include "DeviceVK.h"
#include "RenderPassVK.h"
#include "FrameBufferVK.h"
#include "PipelineVK.h"
#include "SemaphoreVK.h"
#include "SwapChainVK.h"
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

    void CmdVK::draw( VkCommandBuffer cmdBuffer, u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance )
    {
        vkCmdDraw( cmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance );
    }

    VkResult CmdVK::queuePresent( const M<SwapChainVK>& swapChain, u32 imageIndex, const List<M<SemaphoreVK>>& waitSemaphores )
    {
        VkQueue presentQueue = swapChain->device()->presentQueue();
        static thread_local List<VkSemaphore> semaphores;
        semaphores.clear();
        HelperVK::toVkList ( waitSemaphores, semaphores );
        VkPresentInfoKHR presentInfo ={};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = (u32)semaphores.size();
        presentInfo.pWaitSemaphores = semaphores.data();
        presentInfo.swapchainCount = 1;
        VkSwapchainKHR swapChains [] = { swapChain->vk() };
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = (uint32_t*) &imageIndex;
        presentInfo.pResults = NULL;
        return vkQueuePresentKHR( presentQueue, &presentInfo );
    }

}
#include "QueueVK.h"
#include "DeviceVK.h"
#include "FenceVK.h"
#include "SemaphoreVK.h"
#include "SwapChainVK.h"
#include "CommandBufferVK.h"
#include "HelperVK.h"

namespace fv
{
    M<QueueVK> QueueVK::create( VkQueue queue, const M<DeviceVK>& device )
    {
        M<QueueVK> queueVk = std::make_shared<QueueVK>();
        queueVk->m_Queue   = queue;
        queueVk->m_Device  = device;
        return queueVk;
    }

    VkResult QueueVK::submit( const M<CommandBufferVK>& commandBuffer,
                              const M<SemaphoreVK>& waitSemaphore,
                              const M<SemaphoreVK>& signalSemaphore,
                              VkPipelineStageFlags waitDstStageMask,
                              const M<FenceVK>& fenceToSignal )

    {
        VkSubmitInfo submitInfo ={};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitDstStageMask  = &waitDstStageMask;
        submitInfo.waitSemaphoreCount = waitSemaphore ? 1 : 0;
        submitInfo.pWaitSemaphores    = waitSemaphore ? waitSemaphore->vkp() : NULL;
        submitInfo.commandBufferCount = commandBuffer ? 1 : 0;
        submitInfo.pCommandBuffers    = commandBuffer ? commandBuffer->vkp() : NULL;
        submitInfo.signalSemaphoreCount = signalSemaphore ? 1 : 0;
        submitInfo.pSignalSemaphores    = signalSemaphore ? signalSemaphore->vkp() : NULL;
        return vkQueueSubmit( m_Queue, 1, &submitInfo, fenceToSignal ? fenceToSignal->vk() : VK_NULL_HANDLE );
    }

    VkResult QueueVK::present( const M<SwapChainVK>& swapChain, u32 imageIndex, const M<SemaphoreVK>& waitSemaphore )
    {
        VkPresentInfoKHR presentInfo ={};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = waitSemaphore ? 1 : 0;
        presentInfo.pWaitSemaphores = waitSemaphore ? waitSemaphore->vkp() : NULL;
        presentInfo.swapchainCount  = 1;
        VkSwapchainKHR swapChains[] ={ swapChain->vk() };
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = (uint32_t*)&imageIndex;
        presentInfo.pResults = NULL;
        return vkQueuePresentKHR( m_Queue, &presentInfo );
    }

}
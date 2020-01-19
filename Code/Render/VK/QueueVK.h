#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class DeviceVK;
    class SemaphoreVK;
    class SwapChainVK;
    class FenceVK;
    class CommandBufferVK;

    class QueueVK
    {
    public:
        static M<QueueVK> create( VkQueue queue, const M<DeviceVK>& device );

        VkResult submit( const M<CommandBufferVK>& commandBuffer,
                         const M<SemaphoreVK>& waitSemaphore,
                         const M<SemaphoreVK>& signalSemaphore,
                         VkPipelineStageFlags dstWaitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         const M<FenceVK>& fenceToSignalOnDone ={} );

        VkResult present( const M<SwapChainVK>& swapChain, u32 imageIndex, const M<SemaphoreVK>& waitSemaphore );

    private:
        VkQueue m_Queue {};
        W<DeviceVK> m_Device;
    };
}
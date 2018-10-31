#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "IncVulkan.h"

namespace fv
{
    struct FrameObject
    {
    public:
        FrameObject() = default;
        void release(); // Do not put in destructor.
        bool initialize(struct DeviceVK& device, u32 numQueues);

        // In this order. Wait, reset, submit, obtain finished semaphores.
        void waitForFences();
        void resetFences();
        void addCmdBufferToQueue( VkCommandBuffer cb, u32 queueIdx ); // Each thread should have its unique idx ranging form [ 0 to numThreads-1 ]
        void submitCommandBuffers(VkSemaphore waitSemaphore, const Vector<VkQueue>& graphicsQueues);
        const VkSemaphore* finishedSemaphores() const { return m_FinishedSemaphores.data(); }
        u32 numSemaphores() const { return (u32)m_FinishedSemaphores.size(); }

    private:
        struct DeviceVK* m_Device;
        Vector<VkSemaphore> m_FinishedSemaphores;
        Vector<VkFence> m_FrameFences;
        Vector<Vector<VkCommandBuffer>> m_CommandBuffers;
    };
}
#endif
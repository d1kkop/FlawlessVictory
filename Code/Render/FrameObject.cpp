#include "FrameObject.h"
#if FV_VULKAN
#include "HelperVK.h"
#include "DeviceVK.h"
#include "../Core/LogManager.h"

namespace fv
{
    void FrameObject::release()
    {
        if (!m_Device || !m_Device->logical) return;
        u32 numObjects = (u32)m_FinishedSemaphores.size();
        for ( u32 i=0; i<numObjects; ++i )
        {
            vkDestroySemaphore( m_Device->logical, m_FinishedSemaphores[i], nullptr );
            vkDestroyFence( m_Device->logical, m_FrameFences[i], nullptr );
        }
    }

    bool FrameObject::initialize(DeviceVK& device, u32 numQueues)
    {
        assert(device.logical && m_FrameFences.empty() && m_FinishedSemaphores.empty() && numQueues>0);

        m_Device = &device;

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for ( u32 i=0; i<numQueues; ++i )
        {
            VkSemaphore imageFinished;
            VkFence frameFence;

            if (vkCreateSemaphore(device.logical, &semaphoreInfo, nullptr, &imageFinished) != VK_SUCCESS ||
                vkCreateFence(device.logical, &fenceInfo, nullptr, &frameFence) != VK_SUCCESS )
            {
                LOGC("VK Cannot create frame sync objects.");
                return false;
            }
            
            m_FinishedSemaphores.emplace_back( imageFinished );
            m_FrameFences.emplace_back( frameFence );
        }
        m_QueueMutexs.resize( numObjects );
        
        return true;
    }

    void FrameObject::submitCommandBuffers(VkSemaphore waitSemaphore, const Vector<VkQueue>& graphicsQueues)
    {
        assert( /* waitSemaphore allowed to be nullptr */ graphicsQueues.size() == m_FrameFences.size() );
        u32 numObjects = (u32)m_FrameFences.size();
        for ( u32 i=0; i<numObjects; ++i )
        {
            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = { waitSemaphore };
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TRANSFER_BIT };
            submitInfo.waitSemaphoreCount = waitSemaphore ? 1 : 0;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            // Execute command buffers
            submitInfo.commandBufferCount = (u32)m_CommandBuffers[i].size();
            submitInfo.pCommandBuffers = m_CommandBuffers[i].data();

            VkSemaphore signalSemaphores[] = { m_FinishedSemaphores[i] };
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            FV_VKCALL( vkQueueSubmit(graphicsQueues[i], 1, &submitInfo, m_FrameFences[i]) );
        }
    }

    void FrameObject::waitForFences()
    {
        assert( m_Device && m_Device->logical );
        u32 numObjects = (u32)m_FrameFences.size();
        while ( vkWaitForFences(m_Device->logical, numObjects, m_FrameFences.data(), VK_TRUE, (u64)-1) == VK_TIMEOUT );
    }

    void FrameObject::resetFences()
    {
        assert( m_Device && m_Device->logical );
        u32 numObjects = (u32)m_FrameFences.size();
        FV_VKCALL( vkResetFences(m_Device->logical, numObjects, m_FrameFences.data()) );
    }

}
#endif
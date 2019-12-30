#include "CommandBuffersVK.h"
#include "CommandPoolVK.h"
#include "DeviceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    CommandBuffersVK::~CommandBuffersVK()
    {
        if ( m_CommandBuffers.size() )
        {
            vkFreeCommandBuffers( m_CommandPool->device()->logical(), m_CommandPool->vk(), (u32)m_CommandBuffers.size(), m_CommandBuffers.data() );
        }
    }

    M<CommandBuffersVK> CommandBuffersVK::allocate( const M<CommandPoolVK>& pool, u32 numBuffers, bool fromSecondary )
    {
        VkCommandBufferAllocateInfo allocInfo ={};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = pool->vk();
        allocInfo.level = fromSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = numBuffers;
        List<VkCommandBuffer> buffers;
        buffers.resize( numBuffers );
        VK_CALL( vkAllocateCommandBuffers( pool->device()->logical(), &allocInfo, buffers.data() ) );
        auto commBuffersVK = std::make_shared<CommandBuffersVK>();
        commBuffersVK->m_CommandPool = pool;
        commBuffersVK->m_CommandBuffers = buffers;
        return commBuffersVK;
    }

    void CommandBuffersVK::resetBufferState( u32 bufferIdx )
    {
        VK_CALL_VOID( vkResetCommandBuffer( m_CommandBuffers[bufferIdx], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT ) );
    }

    void CommandBuffersVK::resetEntireBufferState()
    {
        for ( u32 i = 0; i < (u32)m_CommandBuffers.size(); i++ )
            resetBufferState( i );
    }

    void CommandBuffersVK::begin( CommandBufferUsage usage, u32 bufferIdx )
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
        VK_CALL_VOID( vkBeginCommandBuffer( m_CommandBuffers[bufferIdx], &beginInfo ) );
    }

    void CommandBuffersVK::end( u32 bufferIdx )
    {
        VK_CALL_VOID( vkEndCommandBuffer( m_CommandBuffers[bufferIdx] ) );
    }

}
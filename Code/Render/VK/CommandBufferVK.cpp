#include "CommandBufferVK.h"
#include "CommandPoolVK.h"
#include "DeviceVK.h"
#include "RenderPassVK.h"
#include "PipelineVK.h"
#include "BufferVK.h"
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

    void CommandBufferVK::endRenderPass()
    {
        vkCmdEndRenderPass( m_CommandBuffer );
    }

    void CommandBufferVK::endRecord()
    {
        VK_CALL_VOID( vkEndCommandBuffer( m_CommandBuffer ) );
    }

}
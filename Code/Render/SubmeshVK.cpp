#include "SubmeshVK.h"
#if FV_VULKAN
#include "DeviceVK.h"
#include "../Core/JobManager.h"

namespace fv
{
    SubmeshVK::~SubmeshVK()
    {
        // Device on BufferVK is always valid even if buffer was not valid.
        device()->deleteCommandBuffers( m_DrawBuffers );
        m_VertexBuffer.release();
        m_IndexBuffer.release();
    }

    SubmeshVK* SubmeshVK::create(DeviceVK& device, const void* vertices, u32 vertexBufferSize, const u32* indices, u32 indexBufferSize)
    {
        VkBufferUsageFlagBits vkUsage = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        VmaMemoryUsage vmaUsage = (VmaMemoryUsage)VMA_MEMORY_USAGE_GPU_ONLY;
        u32 queueIdxs[] = { device.queueIndices.graphics.value() };

        BufferVK vertexBuffer = BufferVK::create(device, vertexBufferSize, vkUsage, vmaUsage, false, queueIdxs, 1, nullptr);
        if ( !vertexBuffer.valid() || !vertexBuffer.copyFrom(vertices, vertexBufferSize) )
        {
            return {};
        }

        BufferVK indexBuffer = BufferVK::create(device, indexBufferSize, vkUsage, vmaUsage, false, queueIdxs, 1, nullptr); 
        if ( !indexBuffer.valid() || !indexBuffer.copyFrom((void*)indices, indexBufferSize) )
        {
            return {};
        }

        auto s = new SubmeshVK{};
        s->m_VertexBuffer = vertexBuffer;
        s->m_IndexBuffer = indexBuffer;

        device.recordCommandBuffers( s->m_DrawBuffers, [s](VkCommandBuffer cb, VkFramebuffer fb)
        {
            s->recordDraw( cb );
        });
        return s;
    }

    void SubmeshVK::recordDraw(VkCommandBuffer cb)
    {
        assert( cb );
        VkBuffer vBuffers [] = { m_VertexBuffer.buffer() };
        VkDeviceSize offsets [] = {0};
        vkCmdBindVertexBuffers( cb, 0, 1, vBuffers, offsets );
        vkCmdBindIndexBuffer( cb, m_IndexBuffer.buffer(), offsets[0], VK_INDEX_TYPE_UINT32 );
        vkCmdDrawIndexed( cb, m_IndexBuffer.size()>>2, 1, 0, 0, 0 );
    }

    void SubmeshVK::addDrawBufferToQueue(FrameObject& fo, u32 queueIdx)
    {
        u32 numThreads = jobManager()->numThreads();
        fo.addCmdBufferToQueue( m_DrawBuffers[fo.idx()*numThreads + queueIdx], queueIdx );
    }

}
#endif
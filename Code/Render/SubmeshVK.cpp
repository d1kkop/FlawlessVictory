#include "SubmeshVK.h"
#include "DeviceVK.h"

namespace fv
{
    SubmeshVK::~SubmeshVK()
    {
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

}
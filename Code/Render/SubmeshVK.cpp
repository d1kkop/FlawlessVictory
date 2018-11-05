#include "SubmeshVK.h"
#if FV_VULKAN
#include "DeviceVK.h"
#include "../Core/JobManager.h"

namespace fv
{
    SubmeshVK::~SubmeshVK()
    {
        m_VertexBuffer.release();
        m_IndexBuffer.release();
    }

    SubmeshVK* SubmeshVK::create(DeviceVK& device, const void* vertices, u32 vertexBufferSize, const u32* indices, u32 indexBufferSize, const SubmeshInput& si, u32 vertexSize)
    {
        VkBufferUsageFlagBits vkUsage = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        VmaMemoryUsage vmaUsage = (VmaMemoryUsage)VMA_MEMORY_USAGE_GPU_ONLY;
        u32 queueIdxs[] = { device.queueIndices.transfer.value() };

        BufferVK vertexBuffer = BufferVK::create(device, vertexBufferSize, vkUsage, vmaUsage, queueIdxs, 1, nullptr);
        if ( !vertexBuffer.valid() || !vertexBuffer.copyFrom(vertices, vertexBufferSize) )
        {
            return {};
        }

        VkBufferUsageFlagBits vkUsageIndex = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        BufferVK indexBuffer = BufferVK::create(device, indexBufferSize, vkUsageIndex, vmaUsage, queueIdxs, 1, nullptr); 
        if ( !indexBuffer.valid() || !indexBuffer.copyFrom((void*)indices, indexBufferSize) )
        {
            return {};
        }

        auto s = new SubmeshVK{};
        s->m_VertexBuffer = vertexBuffer;
        s->m_IndexBuffer = indexBuffer;
        s->m_SubmeshInput = si;
        s->m_VertexSize = vertexSize;
        return s;
    }

    void SubmeshVK::render(VkCommandBuffer cb)
    {
        assert(cb);
        VkBuffer vBuffers[] = { m_VertexBuffer.buffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cb, 0, 1, vBuffers, offsets);
        vkCmdBindIndexBuffer(cb, m_IndexBuffer.buffer(), offsets[0], VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cb, m_IndexBuffer.size()>>2, 1, 0, 0, 0);
    }

}
#endif
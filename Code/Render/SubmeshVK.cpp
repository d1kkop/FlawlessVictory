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

    SubmeshVK* SubmeshVK::create(DeviceVK& device,
                                 const void* vertices, u32 numVertices, u32 vertexSize,
                                 const u32* indices, u32 numIndices,
                                 const SubmeshInput& si)
    {
        assert( vertices && numVertices && indices && numIndices ); // VertexSize can be 0.

        BufferVK vertexBuffer = BufferVK::createVertexBuffer( device, vertices, numVertices, vertexSize );
        if (!vertexBuffer.valid()) return nullptr;

        BufferVK indexBuffer = BufferVK::createIndexBuffer( device, indices, numIndices );
        if (!indexBuffer.valid()) return nullptr;

        auto s = new SubmeshVK{};
        s->m_VertexBuffer = vertexBuffer;
        s->m_IndexBuffer  = indexBuffer;
        s->m_SubmeshInput = si;
        s->m_VertexSize   = vertexSize;
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
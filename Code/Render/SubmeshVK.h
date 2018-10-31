#pragma once
#include "BufferVK.h"

namespace fv
{
    struct SubmeshVK
    {
    public:
        SubmeshVK() = default;
        ~SubmeshVK();
        void deleteSelf();
        static SubmeshVK* create( struct DeviceVK& device, const void* vertices, u32 vertexBufferSize, const u32* indices, u32 indexBufferSize );
        void recordDraw(VkCommandBuffer cb);
        struct DeviceVK* device() const { return m_VertexBuffer.device(); }
        const BufferVK& vertexBuffer() const { return m_VertexBuffer; }
        const BufferVK& indexBuffer() const { return m_IndexBuffer; }

    private:
        BufferVK m_VertexBuffer;
        BufferVK m_IndexBuffer;
        Vector<VkCommandBuffer> m_DrawBuffers;
    };
}
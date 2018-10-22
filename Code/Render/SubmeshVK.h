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
        const BufferVK& vertexBuffer() const { return m_VertexBuffer; }
        const BufferVK& indexBuffer() const { return m_IndexBuffer; }
        struct DeviceVK* device() const { return m_VertexBuffer.device(); }

    private:
        BufferVK m_VertexBuffer;
        BufferVK m_IndexBuffer;
    };
}
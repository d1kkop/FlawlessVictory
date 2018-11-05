#pragma once
#include "BufferVK.h"
#if FV_VULKAN
#include "RenderManagerVK.h"

namespace fv
{
    struct SubmeshVK
    {
    public:
        SubmeshVK() = default;
        ~SubmeshVK();

        static SubmeshVK* create( struct DeviceVK& device, const void* vertices, u32 vertexBufferSize, const u32* indices, u32 indexBufferSize,
                                 const SubmeshInput& si, u32 vertexSize );

        struct DeviceVK* device() const { return m_VertexBuffer.device(); }
        const BufferVK& vertexBuffer() const { return m_VertexBuffer; }
        const BufferVK& indexBuffer() const { return m_IndexBuffer; }
        const SubmeshInput& submeshInput() const { return m_SubmeshInput; }
        u32 vertexSize() const { return m_VertexSize; }

        void render(VkCommandBuffer cb);

    private:
        BufferVK m_VertexBuffer;
        BufferVK m_IndexBuffer;
        SubmeshInput m_SubmeshInput;
        u32 m_VertexSize;
    };
}
#endif
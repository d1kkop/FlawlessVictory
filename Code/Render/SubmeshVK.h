#pragma once
#include "BufferVK.h"
#if FV_VULKAN

namespace fv
{
    struct SubmeshVK
    {
    public:
        SubmeshVK() = default;
        ~SubmeshVK();
        static SubmeshVK* create( struct DeviceVK& device, const void* vertices, u32 vertexBufferSize, const u32* indices, u32 indexBufferSize );
        struct DeviceVK* device() const { return m_VertexBuffer.device(); }
        const BufferVK& vertexBuffer() const { return m_VertexBuffer; }
        const BufferVK& indexBuffer() const { return m_IndexBuffer; }

        void addDrawBufferToQueue(struct FrameObject& fo, u32 queueIdx);

    private:
        void recordDraw(VkCommandBuffer cb);

        BufferVK m_VertexBuffer;
        BufferVK m_IndexBuffer;
        Vector<VkCommandBuffer> m_DrawBuffers;
    };
}
#endif
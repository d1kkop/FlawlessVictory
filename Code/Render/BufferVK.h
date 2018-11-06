#pragma once
#include "MemoryHelperVK.h"

namespace fv
{
    struct DeviceVK;

    struct BufferVK
    {
        BufferVK() = default;
        void release();

        static BufferVK create(DeviceVK& device, u32 size, VkBufferUsageFlagBits usage, VmaMemoryUsage memUsage,
                               const u32* queueIndices, u32 numQueueIndices, void** pMapped);
        static BufferVK createVertexBuffer(DeviceVK& device, const void* vertices, u32 numVertices, u32 vertexSize);
        static BufferVK createIndexBuffer(DeviceVK& device, const u32* indices, u32 numIndices);
        static BufferVK createUniformBuffer(DeviceVK& device, const void* data, u32 size);


        bool map(void** pData);
        void unmap();
        void flush();
        bool copyFrom(const void* pData, u32 size);

        bool valid() const { return m_Valid; }
        VkBuffer buffer() const { return m_Buffer; }
        u32 size() const { return m_Size; }
        DeviceVK* device() const { return m_Device; }

    private:
        bool m_Valid = false;
        DeviceVK* m_Device;
        VkBuffer m_Buffer;
        VmaAllocation m_Allocation;
        VmaAllocator m_Allocator;
        u32 m_Size;
        VmaMemoryUsage m_Usage;
    };
}
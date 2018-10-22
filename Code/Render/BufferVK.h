#pragma once
#include "MemoryHelperVK.h"

namespace fv
{
    struct BufferVK
    {
        BufferVK() = default;
        static BufferVK create(struct DeviceVK& device, u32 size, VkBufferUsageFlagBits usage, VmaMemoryUsage memUsage,
                               bool shareInQueue, const u32* queueIndices, u32 numQueueIndices, void** pMapped);
        void release();
        bool map(void** pData);
        void unmap();
        void flush();
        VkBuffer buffer() const { return m_Buffer; }
        u32 size() const { return m_Size; }
        bool copyFrom(const void* pData, u32 size);
        bool valid() const { return m_Valid; }
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
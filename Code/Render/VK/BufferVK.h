#pragma once
#include "IncVulkan.h"
#include "AllocatorVK.h"
#include "../../Core/Common.h"

namespace fv
{
    class AllocatorVK;

    class BufferVK
    {
    public:
        BufferVK() = default;
        ~BufferVK();

        
        static M<BufferVK> create( const M<AllocatorVK>& allocator,
                                   u32 size,
                                   VkBufferUsageFlags usageFlags,
                                   VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY,
                                   void** createMapped = NULL,
                                   const u32* queueIndices = NULL, u32 numQueues = 0 );

        // If memory was created mapped, you do not have to call this.
        bool map( void** pData );
        void unmap();
        void flush();

        VkBuffer buffer() const { return m_Buffer; }
        VkBuffer vk() const { return buffer(); }

        const M<AllocatorVK>& allocator() const { return m_Allocator; }

    private:
        VkBuffer m_Buffer{};
        VkFormat m_Format{};
        M<AllocatorVK> m_Allocator;
        VmaAllocation  m_Allocation{};
        VmaAllocationInfo m_AllocationInfo{};
        VmaMemoryUsage m_Usage{};
    };
}
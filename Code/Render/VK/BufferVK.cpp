#include "BufferVK.h"
#if FV_VULKAN
#include "AllocatorVK.h"
#include "../Core/LogManager.h"

namespace fv
{
    BufferVK::~BufferVK()
    {
        if ( m_Allocation )
        {
            vmaDestroyBuffer( m_Allocator->vma(), m_Buffer, m_Allocation );
        }
    }

    M<BufferVK> BufferVK::create( const M<AllocatorVK>& allocator,
                                  u32 size,
                                  VkBufferUsageFlags usage,
                                  VmaMemoryUsage memUsage,
                                  void** createMapped,
                                  const u32* queueIndices, u32 numQueues )
    {
        if ( createMapped) *createMapped = NULL;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size  = size;
        bufferInfo.usage = usage;
        bufferInfo.pQueueFamilyIndices = (const uint32_t*)queueIndices;
        bufferInfo.queueFamilyIndexCount = numQueues;
        bufferInfo.sharingMode = numQueues > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = memUsage;
        allocInfo.flags = (createMapped) ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocatedInfo;
        if ( vmaCreateBuffer( allocator->vma(), &bufferInfo, &allocInfo, &buffer, &allocation, &allocatedInfo) != VK_SUCCESS )
        {
            LOGC("VK Failed buffer allocation");
            return {};
        }

        if ( createMapped && !allocatedInfo.pMappedData )
        {
            LOGC("VK Failed buffer allocation as buffer could not be mapped while this was requested.");
            vmaDestroyBuffer( allocator->vma(), buffer, allocation );
            return {};
        }

        if ( createMapped )
        {
            assert(allocatedInfo.pMappedData);
            *createMapped = allocatedInfo.pMappedData;
        }

        M<BufferVK> bufferVK = std::make_shared<BufferVK>();
        bufferVK->m_Allocator = allocator;
        bufferVK->m_Allocation = allocation;
        bufferVK->m_AllocationInfo = allocatedInfo;
        bufferVK->m_Usage  = (VmaMemoryUsage)usage;
        bufferVK->m_Buffer = buffer;
        bufferVK->m_Size = size;
        return bufferVK;
    }

    bool BufferVK::map(void** pData)
    {
        assert(pData);
        if (!pData) 
        {
            LOGC( "VK Invalid pData ptr provided, is NULL." );
            return false;
        }
        VK_CALL (vmaMapMemory(m_Allocator->vma(), m_Allocation, pData) );
        return true;
    }

    void BufferVK::unmap()
    {
        vmaUnmapMemory(m_Allocator->vma(), m_Allocation);
    }

    void BufferVK::flush()
    {
        vmaFlushAllocation( m_Allocator->vma(), m_Allocation, 0, m_AllocationInfo.size );
    }

}
#endif
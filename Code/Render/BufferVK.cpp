#include "BufferVK.h"
#include "DeviceVK.h"
#include "MemoryHelperVK.h"
#include "../Core/LogManager.h"

namespace fv
{
    BufferVK BufferVK::create(DeviceVK& device, u32 size, VkBufferUsageFlagBits usage, VmaMemoryUsage memUsage, 
                              bool shareInQueue, const u32* queueIndices, u32 numQueueIndices, void** pMapped)
    {
        BufferVK bf = {};
        bf.m_Device = &device;

        assert(queueIndices && numQueueIndices >= 1 && size != 0);
        VkBufferCreateInfo bufferInfo {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size  = size;
        bufferInfo.usage = usage;
        bufferInfo.pQueueFamilyIndices = (const uint32_t*)queueIndices;
        bufferInfo.queueFamilyIndexCount = numQueueIndices;
        bufferInfo.sharingMode = shareInQueue ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = memUsage;
        allocInfo.flags = (pMapped) ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

        VmaAllocationInfo allocatedInfo;
        if ( vmaCreateBuffer(device.allocator, &bufferInfo, &allocInfo, &bf.m_Buffer, &bf.m_Allocation, &allocatedInfo) != VK_SUCCESS )
        {
            LOGW("VK Failed buffer allocation");
            return {};
        }

        if ( pMapped && !allocatedInfo.pMappedData )
        {
            LOGW("VK Failed buffer allocation as buffer could not be mapped while this was requested.");
            return {};
        }

        assert(size <= allocatedInfo.size );
        if ( size > allocatedInfo.size )
        {
            LOGW("VK Failed buffer allocation as provided size is smaller than requested.");
            return {};
        }
        
        bf.m_Allocator = device.allocator;
        bf.m_Size = size;
        bf.m_Usage = memUsage;
        if ( pMapped ) 
        {
            assert( allocatedInfo.pMappedData );
            *pMapped = allocatedInfo.pMappedData;
        }
        bf.m_Valid = true;

        return bf;
    }

    bool BufferVK::map(void** pData)
    {
        assert( pData );
        if ( vmaMapMemory(m_Allocator, m_Allocation, pData) == VK_SUCCESS )
            return true;
        return false;
    }

    void BufferVK::unmap()
    {
        vmaUnmapMemory( m_Allocator, m_Allocation );
    }

    void BufferVK::flush()
    {
        vmaFlushAllocation( m_Allocator, m_Allocation, 0, m_Size );
    }

    bool BufferVK::copyFrom(const void* data, u32 size)
    {
        assert( size <= m_Size && data );

        void* pMapped;
        switch ( m_Usage )
        {
        case VMA_MEMORY_USAGE_CPU_ONLY:
        case VMA_MEMORY_USAGE_CPU_TO_GPU:
            if ( map(&pMapped) )
            {
                memcpy( pMapped, data, size );
                unmap();
                flush();
                return true;
            }
            break;

        case VMA_MEMORY_USAGE_GPU_ONLY:
        {
            BufferVK staging;
            void* pStage;
            if ( !m_Device->mapStagingBuffer( staging, &pStage ) )
            {
                return false;
            }
            memcpy(pStage, data, size);
            m_Device->unmapStagingBuffer();

            m_Device->submitOnetimeTransferCommand([&](VkCommandBuffer cb)
            {
                VkBufferCopy region = {};
                region.size = m_Size;
                vkCmdCopyBuffer(cb, staging.buffer(), buffer(), 1, &region);
            });
        }
        break;

        default:
            assert(false);
            LOGC("VK Cannot copy to a buffer that that is a GPU-TO-CPU type");
            return false;
        }

        return true;
    }

    void BufferVK::release()
    {
        vmaDestroyBuffer( m_Allocator, m_Buffer, m_Allocation );
    }

}
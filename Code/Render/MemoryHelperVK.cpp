#include "MemoryHelperVK.h"
#if FV_VULKAN
#define VMA_IMPLEMENTATION
#include "../3rdParty/VulkanAllocator/vk_mem_alloc.h"
#include "DeviceVK.h"
#include "HelperVK.h"
#include "../Core/LogManager.h"

namespace fv
{
    bool MemoryHelperVK::createBuffer(const struct DeviceVK& device, u32 size, 
                                      VkBufferUsageFlagBits usage, VmaMemoryUsage vmaUsage, VkMemoryAllocateFlags flags,
                                      bool shareInQueues, const u32* queueIdxs, u32 numQueueIdxs, BufferVK& ba, void** pMapped)
    {
        assert( queueIdxs && numQueueIdxs >= 1 && size != 0 );
        VkBufferCreateInfo bufferInfo {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.pQueueFamilyIndices = (const uint32_t*) queueIdxs;
        bufferInfo.queueFamilyIndexCount = numQueueIdxs;
        bufferInfo.sharingMode = shareInQueues ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = vmaUsage;
        allocInfo.flags = flags;
        VmaAllocationInfo allocatedInfo;
        if ( vmaCreateBuffer(device.allocator, &bufferInfo, &allocInfo, &ba.buffer, &ba.allocation, &allocatedInfo) != VK_SUCCESS )
        {
            LOGW("VK Failed buffer allocation");
            return false;
        }
        ba.allocator = device.allocator;
        ba.size = size;
        assert( size <= allocatedInfo.size );
        if ( pMapped ) *pMapped = allocatedInfo.pMappedData;
        return true;
    }

    bool MemoryHelperVK::createImage(const struct DeviceVK& device, u32 width, u32 height, VkFormat format,
                                     u32 mipLevels, u32 layers, u32 samples, bool shareInQueues, u32 queueIdx,
                                     VkImageUsageFlagBits usage, VmaMemoryUsage vmaUsage, ImageVK& ia)
    {
        VkImageCreateInfo ici {};
        ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ici.flags = 0;
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.format = format;
        ici.extent = { width, height, 1 };
        ici.mipLevels = mipLevels;
        ici.arrayLayers = layers;
        ici.samples = (VkSampleCountFlagBits)samples;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        ici.usage  = usage;
        ici.sharingMode = shareInQueues ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ici.queueFamilyIndexCount = 1;
        ici.pQueueFamilyIndices = (uint32_t*) &queueIdx;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = vmaUsage;
        if ( vmaCreateImage(device.allocator, &ici, &allocInfo, &ia.image, &ia.allocation, nullptr) != VK_SUCCESS )
        {
            LOGW("VK Failed image allocation");
            return false;
        }
        ia.allocator = device.allocator;
        return true;
    }

    void MemoryHelperVK::freeBuffer(const BufferVK& buffAllocation)
    {
        vmaDestroyBuffer( buffAllocation.allocator, buffAllocation.buffer, buffAllocation.allocation );
    }

    void MemoryHelperVK::freeImage(const ImageVK& imgAllocation)
    {
        vmaDestroyImage( imgAllocation.allocator, imgAllocation.image, imgAllocation.allocation );
    }

    void MemoryHelperVK::copyToStagingBuffer(const struct DeviceVK& device, const void* memory, u32 size)
    {
        assert( device.stagingBuffer && device.stagingMemory );
        memcpy( device.stagingMemory, memory, size );
        // vmaFlushAllocation( device.allocator, device.stagingBuffer->allocation, 0, size );
    }

    VkCommandBuffer MemoryHelperVK::copyBuffer(struct DeviceVK& device, BufferVK& dst, const BufferVK& src)
    {
        /*VkCommandBuffer cb;
        HelperVK::allocCommandBuffer(device.logical, device.commandPool, cb);
        HelperVK::startRecordCommandBuffer(device.logical, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, cb);
*/
        //vkCmdUpdateBuffer( 
        // VkCommandBuffer cb = device.addSingleTimeCmd( [&](VkCommandBuffer cb)
        device.addSingleTimeCmd2( [&](VkCommandBuffer cb)
        {
            VkBufferCopy copyRegion = {};
            copyRegion.size = src.size;
            VkBufferMemoryBarrier bm = {};
            bm.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bm.buffer = dst.buffer;
            bm.size = dst.size;
            bm.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            bm.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
            bm.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bm.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            VkBufferMemoryBarrier bm2 = {};
            bm2.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bm2.buffer = src.buffer;
            bm2.size = src.size;
            bm2.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            bm2.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
            bm2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bm2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            VkBufferMemoryBarrier bms [] = { bm, bm2 };
            vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0,
                                 nullptr, 2, bms, 0, nullptr );

            vkCmdCopyBuffer(cb, src.buffer, dst.buffer, 1, &copyRegion);
        });

        //HelperVK::stopRecordCommandBuffer(cb);

        //VkSubmitInfo submitInfo = {};
        //submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        //submitInfo.commandBufferCount = 1;
        //submitInfo.pCommandBuffers = &cb;

        //FV_VKCALL( vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) );
        //FV_VKCALL( vkQueueWaitIdle(device.graphicsQueue) );

        //HelperVK::freeCommandBuffers(device.logical, device.commandPool, &cb, 1);

        return nullptr;
    }

    VkCommandBuffer MemoryHelperVK::copyToDevice(struct DeviceVK& device, BufferVK& dst, const void* src, u32 size)
    {
        assert( size == dst.size );
        scoped_lock lk(m_CopyToDeviceMtx);
        copyToStagingBuffer( device, src, size );
        return copyBuffer( device, dst, *device.stagingBuffer );
    }

    Mutex MemoryHelperVK::m_CopyToDeviceMtx;

}

#endif
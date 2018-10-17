#include "MemoryHelperVK.h"
#if FV_VULKAN
#define VMA_IMPLEMENTATION
#include "../3rdParty/VulkanAllocator/vk_mem_alloc.h"
#include "DeviceVK.h"
#include "HelperVK.h"
#include "../Core/LogManager.h"

namespace fv
{
    bool MemoryHelperVK::createBuffer(const struct DeviceVK& device, u32 size, VkBufferUsageFlagBits usage, VmaMemoryUsage vmaUsage, BufferVK& ba)

    {
        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = vmaUsage;
        if ( vmaCreateBuffer(device.allocator, &bufferInfo, &allocInfo, &ba.buffer, &ba.allocation, nullptr) != VK_SUCCESS )
        {
            LOGW("VK Failed buffer allocation");
            return false;
        }
        ba.allocator = device.allocator;
        ba.size = size;
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
        ici.extent = { width, width, 1 };
        ici.mipLevels = mipLevels;
        ici.arrayLayers = layers;
        ici.samples = (VkSampleCountFlagBits)samples;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        ici.usage  = usage;
        ici.sharingMode = shareInQueues ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        ici.queueFamilyIndexCount = 1;
        ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t queueFamIndices[] = { queueIdx };
        ici.pQueueFamilyIndices = queueFamIndices;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = vmaUsage;
        if ( vmaCreateImage(device.allocator, &ici, &allocInfo, &ia.image, &ia.allocation, nullptr) != VK_SUCCESS )
        {
            LOGW("VK Failed buffer allocation");
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

    bool MemoryHelperVK::copyToStagingBuffer(const struct DeviceVK& device, const void* memory, u32 size)
    {
        if (!device.stagingBuffer) return false;
        void* pDest;
        if ( vmaMapMemory( device.stagingBuffer->allocator, device.stagingBuffer->allocation, &pDest ) == VK_SUCCESS )
        {
            memcpy( pDest, memory, size );
            vmaUnmapMemory( device.stagingBuffer->allocator, device.stagingBuffer->allocation );
            return true;
        }
        return false;
    }

    bool MemoryHelperVK::copyBuffer(const struct DeviceVK& device, BufferVK& dst, const BufferVK& src)
    {
        Vector<VkCommandBuffer> newCbs;
        if ( !HelperVK::allocCommandBuffers(device.logical, device.commandPool, 1, newCbs) )
        {
            LOGC("Cannot create temporary triangle command buffer.");
            return false;
        }
        if ( !HelperVK::startRecordCommandBuffer(device.logical, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, newCbs[0]) )
        {
            LOGC("VK Failed to start record command buffer.");
            return false;
        }

        VkBufferCopy copyRegion = {};
        copyRegion.size = src.size;
        vkCmdCopyBuffer(newCbs[0], src.buffer, dst.buffer, 1, &copyRegion);

        if ( !HelperVK::stopRecordCommandBuffer(newCbs[0]) )
        {
            LOGC("VK Failed to stop recording command buffer.");
            return false;
        }

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &newCbs[0];

        vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device.graphicsQueue);
        vkFreeCommandBuffers(device.logical, device.commandPool, 1, &newCbs[0]);

        return true;
    }

}

#endif
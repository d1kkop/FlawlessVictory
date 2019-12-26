#include "ImageVK.h"
#if FV_VULKAN
#include "DeviceVK.h"
#include "../Core/LogManager.h"

namespace fv
{
    ImageVK::~ImageVK()
    {
        if ( !m_Device ) return;
        vmaDestroyImage(m_Device->allocator, m_Image, m_Allocation);
    }

    ImageVK* ImageVK::create(DeviceVK& device, u32 width, u32 height, u32 depth, VkFormat format, VkImageTiling tiling,
                             u32 mipLevels, u32 layers, u32 samples, const u32* queueIndices, u32 numQueues,
                             VkImageUsageFlagBits usage, VmaMemoryUsage vmaUsage, void** pMapped)
    {
        assert( device.logical && width > 0 && height > 0 && mipLevels > 0 && layers > 0 && samples > 0 && queueIndices && numQueues > 0 );

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = depth;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = layers;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = (VkSampleCountFlagBits) samples;
        imageInfo.pQueueFamilyIndices = (const uint32_t*)queueIndices;
        imageInfo.queueFamilyIndexCount = numQueues;
        imageInfo.sharingMode = numQueues > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = vmaUsage;
        allocInfo.flags = (pMapped) ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

        VkImage image;
        VmaAllocation allocation;
        VmaAllocationInfo allocatedInfo;
        if ( vmaCreateImage(device.allocator, &imageInfo, &allocInfo, &image, &allocation, &allocatedInfo) != VK_SUCCESS )
        {
            LOGW("VK Failed image allocation");
            return {};
        }

        if ( pMapped && !allocatedInfo.pMappedData )
        {
            LOGW("VK Failed image allocation as image could not be mapped while this was requested.");
            vmaDestroyImage( device.allocator, image, allocation );
            return {};
        }

        if ( pMapped )
        {
            assert(allocatedInfo.pMappedData);
            *pMapped = allocatedInfo.pMappedData;
        }

        ImageVK* img = new ImageVK{};
        img->m_Device = &device;
        img->m_Allocation = allocation;
        img->m_Usage = vmaUsage;
        img->m_Image = image;
        img->m_Width = width;
        img->m_Height = height;
        img->m_Depth = depth;
        img->m_MipLevels = mipLevels;
        img->m_Layers = layers;
        img->m_Format = format;
        return img;
    }

    bool ImageVK::map(void** pData)
    {
        assert(pData);
        if ( vmaMapMemory(m_Device->allocator, m_Allocation, pData) == VK_SUCCESS )
            return true;
        return false;
    }

    void ImageVK::unmap()
    {
        vmaUnmapMemory(m_Device->allocator, m_Allocation);
    }

    void ImageVK::flush()
    {
        vmaFlushAllocation(m_Device->allocator, m_Allocation, 0, m_AllocationInfo.size);
    }

    bool ImageVK::copyFrom(const void* data, u32 size)
    {
        assert(size <= m_AllocationInfo.size && data);

        void* pMapped;
        switch ( m_Usage )
        {
        case VMA_MEMORY_USAGE_CPU_ONLY:
        case VMA_MEMORY_USAGE_CPU_TO_GPU:
        if ( map(&pMapped) )
        {
            memcpy(pMapped, data, size);
            unmap();
            flush();
            return true;
        }
        break;

        case VMA_MEMORY_USAGE_GPU_ONLY:
        {
            //BufferVK staging;
            //void* pStage;
            //if ( !m_Device->mapStagingBuffer(staging, &pStage) )
            //{
            //    return false;
            //}
            //memcpy(pStage, data, size);
            //m_Device->unmapStagingBuffer();

            //m_Device->submitOnetimeTransferCommand([&](VkCommandBuffer cb)
            //{
            //    VkBufferCopy region = {};
            //    region.size = m_Size;
            //    vkCmdCopyImage(
            //    vkCmdCopyBuffer(cb, staging.buffer(), buffer(), 1, &region);
            //});
        }
        break;

        default:
        assert(false);
        LOGC("VK Cannot copy to image that has GPU-TO-CPU flag.");
        return false;
        }

        return true;
    }

}
#endif
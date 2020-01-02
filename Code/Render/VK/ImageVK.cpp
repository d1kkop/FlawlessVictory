#include "ImageVK.h"
#if FV_VULKAN
#include "AllocatorVK.h"
#include "../Core/LogManager.h"

namespace fv
{
    ImageVK::~ImageVK()
    {
        if ( m_Allocation )
        {
            vmaDestroyImage( m_Allocator->vma(), m_Image, m_Allocation );
        }
    }

    M<ImageVK> ImageVK::create( const M<AllocatorVK>& allocator, u32 width, u32 height, u32 depth, VkFormat format,
                                ImageUsageFlagBitsVK usageBit, ImageUsageVK usage,
                                ImageTypeVK imageType,
                                void** createMapped,
                                bool imageTilingLinear,
                                u32 mipLevels, u32 layers, u32 samples,
                                const u32* queueIndices, u32 numQueues)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = (VkImageType) imageType;
        imageInfo.extent.width  = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth  = depth;
        imageInfo.mipLevels   = mipLevels;
        imageInfo.arrayLayers = layers;
        imageInfo.format = format;
        imageInfo.tiling = imageTilingLinear ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = (VkImageUsageFlags)usageBit;
        imageInfo.samples = (VkSampleCountFlagBits) samples;
        imageInfo.pQueueFamilyIndices = (const uint32_t*)queueIndices;
        imageInfo.queueFamilyIndexCount = numQueues;
        imageInfo.sharingMode = numQueues > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = (VmaMemoryUsage) usage;
        allocInfo.flags = (createMapped) ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

        VkImage image;
        VmaAllocation allocation;
        VmaAllocationInfo allocatedInfo;
        if ( vmaCreateImage( allocator->vma(), &imageInfo, &allocInfo, &image, &allocation, &allocatedInfo) != VK_SUCCESS )
        {
            LOGC("VK Failed image allocation");
            return {};
        }

        if ( createMapped && !allocatedInfo.pMappedData )
        {
            LOGC("VK Failed image allocation as image could not be mapped while this was requested.");
            vmaDestroyImage( allocator->vma(), image, allocation );
            return {};
        }

        if ( createMapped )
        {
            assert(allocatedInfo.pMappedData);
            *createMapped = allocatedInfo.pMappedData;
        }

        M<ImageVK> imageVK = std::make_shared<ImageVK>();
        imageVK->m_Allocator = allocator;
        imageVK->m_Allocation = allocation;
        imageVK->m_AllocationInfo = allocatedInfo;
        imageVK->m_Usage = (VmaMemoryUsage)usage;
        imageVK->m_Image = image;
        imageVK->m_Width = width;
        imageVK->m_Height = height;
        imageVK->m_Depth  = depth;
        imageVK->m_MipLevels = mipLevels;
        imageVK->m_Layers = layers;
        imageVK->m_Format = format;
        return imageVK;
    }

    bool ImageVK::map(void** pData)
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

    void ImageVK::unmap()
    {
        vmaUnmapMemory(m_Allocator->vma(), m_Allocation);
    }

    void ImageVK::flush()
    {
        vmaFlushAllocation(m_Allocator->vma(), m_Allocation, 0, m_AllocationInfo.size);
    }

}
#endif
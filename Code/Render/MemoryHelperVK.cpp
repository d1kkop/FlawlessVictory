#include "MemoryHelperVK.h"
#include "BufferVK.h"
#if FV_VULKAN
#define VMA_IMPLEMENTATION
#include "../3rdParty/VulkanAllocator/vk_mem_alloc.h"
#include "DeviceVK.h"
#include "HelperVK.h"
#include "../Core/LogManager.h"

namespace fv
{
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

    void MemoryHelperVK::freeImage(const ImageVK& imgAllocation)
    {
        vmaDestroyImage( imgAllocation.allocator, imgAllocation.image, imgAllocation.allocation );
    }
}

#endif
#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "IncVulkan.h"
#include "../3rdParty/VulkanAllocator/vk_mem_alloc.h"

namespace fv
{
    struct ImageVK
    {
        VkImage image;
        VmaAllocation allocation;
        VmaAllocator allocator;
    };

    class MemoryHelperVK
    {
    public:
        static bool createImage(const struct DeviceVK& device, u32 width, u32 height, VkFormat format,
                                u32 mipLevels, u32 layers, u32 samples, bool shareInQueues, u32 queueIdx,
                                VkImageUsageFlagBits usage, VmaMemoryUsage vmaUsage, ImageVK& buffAllocation);
        static void freeImage(const ImageVK& imgAllocation);

    };
}
#endif

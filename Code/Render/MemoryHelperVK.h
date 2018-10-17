#pragma once
#include "../Core/Common.h"
#if FV_VULKAN

namespace fv
{
    struct BufferVK
    {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocator allocator;
        u32 size;
    };

    struct ImageVK
    {
        VkImage image;
        VmaAllocation allocation;
        VmaAllocator allocator;
    };

    class MemoryHelperVK
    {
    public:
        static bool createBuffer(const struct DeviceVK& device, u32 size, VkBufferUsageFlagBits usage, VmaMemoryUsage vmaUsage, BufferVK& buffAllocation);
        static bool createImage(const struct DeviceVK& device, u32 width, u32 height, VkFormat format,
                                u32 mipLevels, u32 layers, u32 samples, bool shareInQueues, u32 queueIdx,
                                VkImageUsageFlagBits usage, VmaMemoryUsage vmaUsage, ImageVK& buffAllocation);
        static void freeBuffer(const BufferVK& buffAllocation);
        static void freeImage(const ImageVK& imgAllocation);
        static void copyToStagingBuffer( const struct DeviceVK& device, const void* memory, u32 size );
        static void copyBuffer( const struct DeviceVK& device, BufferVK& dst, const BufferVK& src );

    };
}
#endif

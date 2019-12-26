#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "IncVulkan.h"

namespace fv
{
    struct DeviceVK;

    struct ImageVK
    {
        ImageVK() = default;
        ~ImageVK();

        static ImageVK* create(DeviceVK& device, u32 width, u32 height, u32 depth, VkFormat format, VkImageTiling tiling,
                               u32 mipLevels, u32 layers, u32 samples, const u32* queueIndices, u32 numQueues,
                               VkImageUsageFlagBits usage, VmaMemoryUsage vmaUsage, void** pMapped);

        bool map(void** pData);
        void unmap();
        void flush();
        bool copyFrom(const void* pData, u32 size);

        DeviceVK* device() const { return m_Device; }
        VkImage image() const { return m_Image; }
        u32 width() const { return m_Width; }
        u32 height() const { return m_Height; }
        u32 depth() const { return m_Depth; }
        u32 mipLevels() const { return m_MipLevels; }
        u32 layers() const { return m_Layers; }
        VkFormat format() const { return m_Format; }

    private:
        DeviceVK* m_Device;
        VkImage m_Image;
        VmaAllocation m_Allocation;
        VmaAllocationInfo m_AllocationInfo;
        VmaMemoryUsage m_Usage;
        u32 m_Width, m_Height, m_Depth;
        u32 m_MipLevels, m_Layers;
        VkFormat m_Format;
    };
}
#endif
#pragma once
#include "IncVulkan.h"
#include "AllocatorVK.h"
#include "../../Core/Common.h"

namespace fv
{
    class AllocatorVK;

    class ImageVK
    {
    public:
        ImageVK() = default;
        ~ImageVK();

        static M<ImageVK> create( const M<AllocatorVK>& allocator, u32 width, u32 height, u32 depth, VkFormat format,
                                  VkImageUsageFlags imageUsage, VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY,
                                  VkImageType imgageType = VK_IMAGE_TYPE_2D,
                                  void** createMapped = NULL,
                                  bool layoutLinearForBufferAccess = false, /* If true, pixels can be indexed linearly (y*width+x), otherwise layout_optimal is picked. */
                                  u32 mipLevels = 100, u32 layers = 1, u32 samples = 1,
                                  const u32* queueIndices = NULL, u32 numQueues = 0 );

        // If memory was created mapped, you do not have to call this.
        bool map( void** pData );
        void unmap();
        void flush();

        VkImage image() const { return m_Image; }
        VkImage vk() const { return image(); }
        u32 width() const { return m_Width; }
        u32 height() const { return m_Height; }
        u32 depth() const { return m_Depth; }
        u32 mipLevels() const { return m_MipLevels; }
        u32 layers() const { return m_Layers; }
        VkFormat format() const { return m_Format; }
        const M<AllocatorVK>& allocator() const { return m_Allocator; }

    private:
        VkImage m_Image{};
        VkFormat m_Format{};
        M<AllocatorVK> m_Allocator;
        VmaAllocation  m_Allocation{};
        VmaAllocationInfo m_AllocationInfo{};
        VmaMemoryUsage m_Usage{};
        u32 m_Width = 0, m_Height = 0, m_Depth = 0;
        u32 m_MipLevels = 0, m_Layers = 0;
    };
}
#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    enum class ViewTypeVK
    {
        Type1D,
        Type2D,
        Type3D,
        TypeCube,
        Type2DArray,
        TypeCubeArray
    };

    enum class AspectTypeVK
    {
        Color,
        Stencil,
        Depth
    };

    class DeviceVK;

    class ImageViewVK
    {
    public:
        ImageViewVK() = default;
        ~ImageViewVK();

    public:
        static M<ImageViewVK> create( const M<DeviceVK>& device,
                                      VkImage image,
                                      VkFormat format, 
                                      ViewTypeVK viewType,
                                      AspectTypeVK aspectType, 
                                      u32 baseArrayLayer=0, u32 layerCount=1, 
                                      u32 mipmapBaseLevel=0, u32 levelCount=1 );

        VkImageView vk() const { return m_ImageView; }

    private:
        VkImageView m_ImageView {};
        M<DeviceVK> m_Device;
    };
}
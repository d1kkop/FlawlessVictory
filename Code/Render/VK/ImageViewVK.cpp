#include "ImageViewVK.h"
#include "DeviceVK.h"
#include "ImageVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    ImageViewVK::~ImageViewVK()
    {
        if ( m_ImageView )
        {
            vkDestroyImageView( m_Device->logical(), m_ImageView, NULL );
        }
    }

    M<ImageViewVK> ImageViewVK::create( const M<DeviceVK>& device,
                                        VkImage image,
                                        VkFormat format,
                                        ViewTypeVK viewType,
                                        AspectTypeVK aspectType,
                                        u32 baseArrayLayer, u32 layerCount,
                                        u32 mipmapBaseLevel, u32 levelCount )
    {
        VkImageView imageView;
        VkImageViewCreateInfo createInfo ={};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = (VkImageViewType) viewType;
        createInfo.format = format;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = (VkImageAspectFlags) aspectType;
        createInfo.subresourceRange.baseMipLevel = mipmapBaseLevel;
        createInfo.subresourceRange.levelCount = levelCount;
        createInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
        createInfo.subresourceRange.layerCount = layerCount;

        if ( vkCreateImageView( device->logical(), &createInfo, nullptr, &imageView ) != VK_SUCCESS )
        {
            LOGC( "VK failed to create image view." );
            return false;
        }

        auto imageViewVK = std::make_shared<ImageViewVK>();
        imageViewVK->m_ImageView = imageView;
        imageViewVK->m_Device = device;
        return imageViewVK;
    }


    M<ImageViewVK> ImageViewVK::create( const M<DeviceVK>& device, 
                                        const M<ImageVK> image, 
                                        VkFormat format, 
                                        ViewTypeVK viewType,
                                        AspectTypeVK aspectType,
                                        u32 baseArrayLayer, 
                                        u32 layerCount,
                                        u32 mipmapBaseLevel, 
                                        u32 levelCount )
    {
        M<ImageViewVK> imgView = create( device, image->vk(), format, viewType, aspectType, baseArrayLayer, layerCount, mipmapBaseLevel, levelCount );
        imgView->m_Image = image;
        return imgView;
    }

}
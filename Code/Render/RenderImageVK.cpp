#include "RenderImageVK.h"
#if FV_VULKAN
#include "RenderManager.h" // For config
#include "DeviceVK.h"
#include "HelperVK.h"
#include "ImageVK.h"

namespace fv
{
    bool RenderImageVK::initialize(DeviceVK& device, VkRenderPass renderPass, const Function<bool (u32, VkImage& img, VkFormat& format, VkImageAspectFlags& flags, u32& layers)>& imagesCb)
    {
        m_Device = &device;
        m_ImageViews.clear();

        bool bContinue = false;
        u32 i=0;
        do
        {
            VkImage img;
            VkFormat format;
            u32 layers;
            VkImageAspectFlags aspectFlags;
            bContinue = imagesCb( i++, img, format, aspectFlags, layers );
            // Create img view for each attached image
            VkImageView imgView;
            if ( !HelperVK::createImageView(m_Device->logical, img, format, layers, aspectFlags, imgView) )
            {
                for ( auto& iv : m_ImageViews ) vkDestroyImageView( device.logical, iv, nullptr );
            }
            m_ImageViews.emplace_back( imgView );
        } while (bContinue);

        if ( !HelperVK::createFramebuffer(m_Device->logical, m_Device->extent, renderPass, m_ImageViews, m_FrameBuffer) )
        {
            return false;
        }

        return true;
    }

    void RenderImageVK::release()
    {
        if (!m_Device ||!m_Device->logical) return;
        for ( auto& vi : m_ImageViews ) vkDestroyImageView( m_Device->logical, vi, nullptr );
        vkDestroyFramebuffer( m_Device->logical, m_FrameBuffer, nullptr );
    }

}
#endif
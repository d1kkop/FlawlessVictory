#include "SwapChainVK.h"
#include "DeviceVK.h"
#include "SurfaceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"
#include "../../Core/Functions.h"
#include "../../Core/IncGLFW.h"

namespace fv
{
    SwapChainVK::~SwapChainVK()
    {
    #if FV_INCLUDE_WINHDR
        if ( m_SwapChain )
        {
            vkDestroySwapchainKHR( m_Device->logical(), m_SwapChain, NULL );
        }
    #else
    #error no impl
    #endif
    }

    M<SwapChainVK> SwapChainVK::create( const M<DeviceVK>& device,
                                        const M<SurfaceVK>& surface,
                                        u32 width, u32 height, u32 numImages, u32 numLayers,
                                        const Set<u32>& queueFamIndicesToAccess )
    {
        VkSurfaceFormatKHR chosenFormat;
        VkPresentModeKHR chosenPresentMode;
        VkExtent2D chosenExtent;
        Vector<VkPresentModeKHR> presentModes;
        Vector<VkSurfaceFormatKHR> formats;
        VkSurfaceCapabilitiesKHR capabilities;
        querySwapChainInfo( device->physical(), surface->vk(), formats, capabilities, presentModes );
        if ( !chooseSwapChain1( width, height, formats, capabilities, presentModes, chosenFormat, chosenPresentMode, chosenExtent ) )
        {
            return {};
        }

        u32 imageCount = Max<u32>( numImages, capabilities.minImageCount );
        if ( capabilities.maxImageCount != 0 ) // Only clamp to max if specified. Some GPU's do not specify.
        {
            imageCount = Min<u32>( imageCount, capabilities.maxImageCount );
        }
        u32 imageArrayLayerCount = Clamp<u32>( numLayers, 1U, (u32)capabilities.maxImageArrayLayers );

        VkSwapchainCreateInfoKHR createInfo ={};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface->vk();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = chosenFormat.format;
        createInfo.imageColorSpace = chosenFormat.colorSpace;
        createInfo.imageExtent = chosenExtent;
        createInfo.imageArrayLayers = imageArrayLayerCount; // In case of 3d stereo rendering must be 2
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if ( queueFamIndicesToAccess.size() > 1 )
        {
            List<u32> indicesAsList;
            for ( auto& idx : queueFamIndicesToAccess ) indicesAsList.push_back( idx );
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = (u32)indicesAsList.size();
            createInfo.pQueueFamilyIndices = (uint32_t*)indicesAsList.data();
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = capabilities.currentTransform;   // Pre transform the image (eg flip horizontal)
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // If want to blend with other windows in system
        createInfo.presentMode = chosenPresentMode;
        createInfo.clipped = VK_TRUE; // Whether hidden pixels by other windows are obscured
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VkSwapchainKHR swapChain {};
        if ( vkCreateSwapchainKHR( device->logical(), &createInfo, nullptr, &swapChain ) != VK_SUCCESS )
        {
            LOGC( "VK Failed to create swap chain for device." );
            return {};
        }

        auto swapChainVk = std::make_shared<SwapChainVK>();
        swapChainVk->m_Device = device;
        swapChainVk->m_Surface = surface;
        swapChainVk->m_SwapChain = swapChain;
        swapChainVk->m_Format = chosenFormat;
        swapChainVk->m_Extent = chosenExtent;

        VK_CALL( vkGetSwapchainImagesKHR( device->logical(), swapChain, (uint32_t*) &numImages, nullptr ) );
        swapChainVk->m_Images.resize( numImages );
        VK_CALL( vkGetSwapchainImagesKHR( device->logical(), swapChain, (uint32_t*) &numImages, swapChainVk->m_Images.data() ) );

        return swapChainVk;
    }

    void SwapChainVK::querySwapChainInfo( VkPhysicalDevice device, VkSurfaceKHR surface, 
                                          Vector<VkSurfaceFormatKHR>& formats, 
                                          VkSurfaceCapabilitiesKHR& capabilities, 
                                          Vector<VkPresentModeKHR>& presentModes )
    {
        assert( device && surface );
        uint32_t formatCount;
        uint32_t presentModeCount;
        VK_CALL_VOID( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, surface, &capabilities ) );
        VK_CALL_VOID( vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &formatCount, nullptr ) );
        if ( formatCount > 0 )
        {
            formats.resize( formatCount );
            VK_CALL_VOID( vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &formatCount, formats.data() ) );
        }
        VK_CALL_VOID( vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &presentModeCount, nullptr ) );
        if ( presentModeCount > 0 )
        {
            presentModes.resize( presentModeCount );
            VK_CALL_VOID( vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &presentModeCount, presentModes.data() ) );
        }
    }

    bool SwapChainVK::chooseSwapChain1( u32 width, u32 height, const Vector<VkSurfaceFormatKHR>& formats, 
                                        const VkSurfaceCapabilitiesKHR& capabilities, const Vector<VkPresentModeKHR>& presentModes,
                                        VkSurfaceFormatKHR& format, VkPresentModeKHR& mode, VkExtent2D& extent )
    {
        bool found = false;
        if ( formats.size() > 0 )
        {
            if ( formats[0].format == VK_FORMAT_UNDEFINED )
            {
                format ={ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
                found = true;
            }
            else
            {
                for ( const auto& f : formats )
                {
                    if ( f.format == VK_FORMAT_B8G8R8A8_UNORM  && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
                    {
                        format = f;
                        found = true;
                        break;
                    }
                }
            }
        }
        if ( !found )
        {
            return false;
        }
        mode = VK_PRESENT_MODE_FIFO_KHR;
        for ( const auto& presentMode : presentModes )
        {
            if ( presentMode == VK_PRESENT_MODE_MAILBOX_KHR )
            {
                mode = presentMode;
                break;
            }
            else if ( presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR )
            {
                mode = presentMode;
            }
        }
        if ( capabilities.currentExtent.width != UINT_MAX )
        {
            extent = capabilities.currentExtent;
        }
        else
        {
            extent ={ width, height };
            extent.width  = Clamp( extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
            extent.height = Clamp( extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height );
        }
        if ( extent.width < 1 || extent.height < 1 )
        {
            return false;
        }
        return true;
    }

}
#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class DeviceVK;
    class SurfaceVK;

    class SwapChainVK
    {
    public:
        SwapChainVK() = default;
        ~SwapChainVK();

    public:
        static M<SwapChainVK> create( const M<DeviceVK>& device,
                                      const M<SurfaceVK>& surface,
                                      u32 width, u32 height, u32 numImages, u32 numLayers,
                                      const List<u32>& queueFamIndicesToAccess );

        static void querySwapChainInfo( VkPhysicalDevice device, VkSurfaceKHR surface,
                                        Vector<VkSurfaceFormatKHR>& formats,
                                        VkSurfaceCapabilitiesKHR& capabilities,
                                        Vector<VkPresentModeKHR>& presentModes );

        /* Add possibly other swapChain pickers. This is just a way to pick one. */
        static bool chooseSwapChain1( u32 width, u32 height,
                                      const Vector<VkSurfaceFormatKHR>& formats,
                                      const VkSurfaceCapabilitiesKHR& capabilities,
                                      const Vector<VkPresentModeKHR>& presentModes,
                                      VkSurfaceFormatKHR& format, VkPresentModeKHR& mode, VkExtent2D& extent );

        VkSwapchainKHR vk() const { return m_SwapChain; }
        const M<DeviceVK>& device() const { return m_Device; }
        const M<SurfaceVK>& surface() const { return m_Surface; }
        VkSurfaceFormatKHR format() const { return m_Format; }
        const VkExtent2D& extent() const { return m_Extent; }
        u32 numImages() const { return (u32)m_Images.size(); }
        VkImage image(u32 idx) const { return m_Images[idx]; }

    private:
        VkSwapchainKHR m_SwapChain;
        M<DeviceVK>  m_Device;
        M<SurfaceVK> m_Surface;
        List<VkImage> m_Images;
        VkSurfaceFormatKHR m_Format {};
        VkExtent2D m_Extent {};
    };
}
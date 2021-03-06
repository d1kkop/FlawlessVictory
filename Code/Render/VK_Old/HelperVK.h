#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "IncVulkan.h"

#if FV_DEBUG
#define FV_VKCALL( exp ) assert( exp==VK_SUCCESS )
#else
#define FV_VKCALL( exp ) exp
#endif

namespace fv
{
    struct QueueFamilyIndicesVK;

    using DebugCallbackVK = VKAPI_ATTR VkBool32 (VKAPI_CALL*)
        (VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);


    class HelperVK
    {
    public:
        // Creation of vk objects.
        static bool createInstance(const String& appName,const Vector<const char*>& requiredExtensions, const Vector<const char*>& requiredLayers, VkInstance& instance);
        static bool createDebugCallback(VkInstance instance, bool showVerbose, bool showInfo, DebugCallbackVK cb, VkDebugUtilsMessengerEXT& debugCallback);
        static bool createDevice(VkInstance instance, VkPhysicalDevice physical, const Vector<VkDeviceQueueCreateInfo>& queueCreateInfos, 
                                 const Vector<const char*>& requiredExtensions, const Vector<const char*>& requiredLayers, VkDevice& logical);
        static bool createSurface(VkInstance instance, const void* wHandle, VkSurfaceKHR& surface);
        static bool createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                    u32 width, u32 height, u32 numImages, u32 numLayers,
                                    const Optional<u32>& graphicsQueueIdx, const Optional<u32>& presentQueueIdx,
                                    VkSurfaceFormatKHR& chosenFormat, VkPresentModeKHR& chosenPresentMode,
                                    VkExtent2D& surfaceExtend, VkSwapchainKHR oldSwapChain, VkSwapchainKHR& swapChain);
        static bool createImageView(VkDevice device, VkImage image, VkFormat format, u32 numLayers, VkImageAspectFlags aspectFlags, VkImageView& imgView);
        static bool createShaderFromBinary(VkDevice device, const Path& path, VkShaderModule& shaderModule);
        static bool createShaderModule(VkDevice device, const char* data, u32 size, VkShaderModule& shaderModule);
        static bool createShaderModule(VkDevice device, const Vector<char>& code, VkShaderModule& shaderModule);
        static bool createPipeline(VkDevice device,
                                   const struct PipelineFormatVK& format,
                                   const VkViewport& vp,
                                   const Vector<VkVertexInputBindingDescription>& vertexBindings,
                                   const Vector<VkVertexInputAttributeDescription>& vertexAttribs,
                                   VkPipeline& pipeline,
                                   VkPipelineLayout& pipelineLayout);
        static bool createFramebuffer(VkDevice device, const VkExtent2D& size, VkRenderPass renderPass, const Vector<VkImageView>& attachments, VkFramebuffer& framebuffer);
        static bool createCommandPool(VkDevice device, u32 familyQueueIndex, VkCommandPool& pool);
        static void createVertexAttribs(const struct SubmeshInput& sinput, Vector<VkVertexInputAttributeDescription>& inputAttribs, u32& vertexSize);

        // Command buffers
        static void allocCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer& commandBuffer);
        static void allocCommandBuffers(VkDevice device, VkCommandPool commandPool, u32 numCommandBuffers, Vector<VkCommandBuffer>& commandBuffers);
        static void beginCommandBuffer(VkDevice device, VkCommandBufferUsageFlags usage, VkCommandBuffer commandBuffer);
        static void startRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer frameBuffer, const VkRect2D& renderArea, const VkClearValue* clearValue, u32 numClearValues);
        static void stopRenderPass(VkCommandBuffer commandBuffer);
        static void endCommandBuffer(VkCommandBuffer commandBuffer);
        static void freeCommandBuffers(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* buffers, u32 numBuffers);
       
        // Checks and validation.
        static void queryRequiredWindowsExtensions(void* pWindow, Vector<const char*>& listToFill);
        static bool checkRequiredLayers(const Vector<const char*>& requiredList, VkPhysicalDevice physicalDevice=nullptr);
        static bool checkRequiredExtensions(const Vector<const char*>& requiredList, VkPhysicalDevice physicalDevice=nullptr);
        static void queryExtensions(Vector<String>& extensions, VkPhysicalDevice physicalDevice=nullptr);
        static void queryLayers(Vector<String>& layers, VkPhysicalDevice physicalDevice=nullptr);
        static bool validateNameList(const Vector<String>& found, const Vector<const char*>& required);
        static void getQueueIndices(VkPhysicalDevice physical, VkSurfaceKHR surface, QueueFamilyIndicesVK& queueIndices);

        // Swap chain query and pick.
        static void querySwapChainInfo(VkPhysicalDevice device, VkSurfaceKHR surface, Vector<VkSurfaceFormatKHR>& formats,
                                       VkSurfaceCapabilitiesKHR& capabilities, Vector<VkPresentModeKHR>& presentModes);
        static bool chooseSwapChain(u32 width, u32 height,
                                    const Vector<VkSurfaceFormatKHR>& formats, const VkSurfaceCapabilitiesKHR& capabilities, const Vector<VkPresentModeKHR>& presentModes,
                                    VkSurfaceFormatKHR& format, VkPresentModeKHR& mode, VkExtent2D& extent);

        // Misc
        static u32 findMemoryType(u32 typeFilter, const VkPhysicalDeviceMemoryProperties& memProperties, VkMemoryPropertyFlags propertyFlags);
        static VkFormat convert( enum class ImageFormat imgFormat );
        static VkFormat findSupportedFormat(VkPhysicalDevice device, const Vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        static VkFormat findDepthFormat(VkPhysicalDevice device);
    };
}
#endif

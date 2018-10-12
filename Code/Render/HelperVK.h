#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include <vulkan/vulkan.h>

namespace fv
{
    struct QueueFamilyIndicesVK
    {
        Optional<u32> graphics;
        Optional<u32> compute;
        Optional<u32> transfer;
        Optional<u32> sparse;
        Optional<u32> present;
        bool complete() const { return graphics.has_value() && compute.has_value() && transfer.has_value() && sparse.has_value() && present.has_value(); }
    };

    struct DeviceVK
    {
        VkDevice logical;
        VkPhysicalDevice physical;
        VkQueue graphicsQueue;
        VkQueue computeQueue;
        VkQueue transferQueue;
        VkQueue sparseQueue;
        VkQueue presentQueue;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memProperties;
        VkPhysicalDeviceFeatures features;
        QueueFamilyIndicesVK queueIndices;
        VkPipeline opaquePipeline;
        VkPipelineLayout opaquePipelineLayout;
        VkShaderModule standardFrag;
        VkShaderModule standardVert;
        VkRenderPass clearPass;
        VkCommandPool commandPool;
        VkExtent2D extent;
        VkFormat format;
        Vector<VkImage> images;
        Vector<VkImageView> imgViews;
        Vector<VkFramebuffer> frameBuffers;
        Vector<VkCommandBuffer> commandBuffers;
        Vector<VkFence> frameFences;
        Vector<VkSemaphore> imageAvailableSemaphores;
        Vector<VkSemaphore> imageFinishedSemaphores;
        struct SwapChainVK* swapChain;
    };

    struct SwapChainVK
    {
        DeviceVK* device;
        VkSurfaceKHR surface;
        VkExtent2D extent;
        VkPresentModeKHR presentMode;
        VkSurfaceFormatKHR surfaceFormat;
        VkSwapchainKHR swapChain;
        VkSwapchainKHR oldSwapChain;
    };

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
                                    const struct RenderConfig& rc, const Optional<u32>& graphicsQueueIdx, const Optional<u32>& presentQueueIdx,
                                    VkSurfaceFormatKHR& chosenFormat, VkPresentModeKHR& chosenPresentMode,
                                    VkExtent2D& surfaceExtend, VkSwapchainKHR& swapChain);
        static bool createImageView(VkDevice device, VkImage image, VkFormat format, u32 numLayers, VkImageView& imgView);
        static bool createShaderFromBinary(VkDevice device, const Path& path, VkShaderModule& shaderModule);
        static bool createShaderModule(VkDevice device, const Vector<char>& code, VkShaderModule& shaderModule);
        static bool createRenderPass(VkDevice device, VkFormat format, VkRenderPass& renderPass);
        static bool createPipeline(VkDevice device, VkShaderModule vertShader, VkShaderModule fragShader, VkRenderPass renderPass,
                                   VkExtent2D vpSize, VkPipelineLayout& pipelineLayout, VkPipeline& pipeline);
        static bool createFramebuffer(VkDevice device, const VkExtent2D& size, VkRenderPass renderPass, const Vector<VkImageView>& attachments, VkFramebuffer& framebuffer);
        static bool createCommandPool(VkDevice device, u32 familyQueueIndex, VkCommandPool& pool);
        

        // Command buffers
        static bool allocCommandBuffers(VkDevice device, VkCommandPool commandPool, u32 numCommandBuffers, Vector<VkCommandBuffer>& commandBuffers);
        static bool startRecordCommandBuffer(VkDevice device, VkCommandBuffer commandBuffer);
        static void startRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer frameBuffer, const VkRect2D& renderArea, const VkClearValue* clearVal);
        static void stopRenderPass(VkCommandBuffer commandBuffer);
        static bool stopRecordCommandBuffer(VkCommandBuffer commandBuffer);
       
        // Checks and validation.
        static void queryRequiredWindowsExtensions(void* pWindow, Vector<const char*>& listToFill);
        static bool checkRequiredLayers(const Vector<const char*>& requiredList, VkPhysicalDevice physicalDevice=nullptr);
        static bool checkRequiredExtensions(const Vector<const char*>& requiredList, VkPhysicalDevice physicalDevice=nullptr);
        static void queryExtensions(Vector<String>& extensions, VkPhysicalDevice physicalDevice=nullptr);
        static void queryLayers(Vector<String>& layers, VkPhysicalDevice physicalDevice=nullptr);
        static bool validateNameList(const Vector<String>& found, const Vector<const char*>& required);

        // Swap chain query and pick.
        static void querySwapChainInfo(VkPhysicalDevice device, VkSurfaceKHR surface, Vector<VkSurfaceFormatKHR>& formats,
                                       VkSurfaceCapabilitiesKHR& capabilities, Vector<VkPresentModeKHR>& presentModes);
        static bool chooseSwapChain(u32 width, u32 height,
                                    const Vector<VkSurfaceFormatKHR>& formats, const VkSurfaceCapabilitiesKHR& capabilities, const Vector<VkPresentModeKHR>& presentModes,
                                    VkSurfaceFormatKHR& format, VkPresentModeKHR& mode, VkExtent2D& extend);

        // Compound helpers. Stores data directly in DeviceVK.
        static void storeDeviceQueueFamilies(DeviceVK& device, VkSurfaceKHR mainSurface);

        // Misc
        static u32  findMemoryType(u32 typeFilter, const VkPhysicalDeviceMemoryProperties& memProperties, VkMemoryPropertyFlags propertyFlags);
    };
}
#endif

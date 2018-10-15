#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include <vulkan/vulkan.h>

namespace fv
{
    enum VertexDescriptorType
    {
        P,                  /* pos */
        PN,                 /* pos normal */
        PNT,                /* pos normal tex */
        PNTL,               /* pos normal tex lightuv */
        PNTLTB,             /* pos normal tex lightuv tan&bin */
        PNTTB,              /* pos normal tex t&b */
        PNTTBBO,            /* pos normal tex t&b bones */
        PNTBO,              /* pos normal tex bones */
        PNBO,               /* pos normal bones */
        PBO                 /* pos normal bones */
    };

    struct QueueFamilyIndicesVK
    {
        Optional<u32> graphics;
        Optional<u32> compute;
        Optional<u32> transfer;
        Optional<u32> sparse;
        Optional<u32> present;
        bool complete() const { return graphics.has_value() && compute.has_value() && transfer.has_value() && sparse.has_value() && present.has_value(); }
    };

    struct PipelineVK
    {
        VkPipeline pipeline;
        VkPipelineLayout layout;
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
        VkShaderModule standardFrag;
        VkShaderModule standardVert;
        VkRenderPass clearPass;
        VkCommandPool commandPool;
        VkExtent2D extent;
        VkFormat format;
        Vector<VkImage> m_Textures2D;
        Vector<VkShaderModule> m_Shaders;
        Vector<VkBuffer> m_Submeshes;
        Vector<VkBuffer> m_SubMeshIndices;
        Vector<VkImage> images;
        Vector<VkImageView> imgViews;
        Vector<VkFramebuffer> frameBuffers;
        Vector<VkCommandBuffer> commandBuffers;
        Vector<VkFence> frameFences;
        Vector<VkSemaphore> imageAvailableSemaphores;
        Vector<VkSemaphore> imageFinishedSemaphores;
        // Dynamically created possible at a later stage
        Map<u32, PipelineVK> pipelines;
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
        static bool createRenderPass(VkDevice device, VkFormat format, u32 samples, VkRenderPass& renderPass);
        static bool createPipeline(VkDevice device, VkShaderModule vertShader, VkShaderModule fragShader, VkShaderModule geomShader, VkRenderPass renderPass, const VkViewport& vp,
                                   const Vector<VkVertexInputAttributeDescription>& vertexInputs, u32 totalVertexSize,
                                   VkPipeline& pipeline, VkPipelineLayout& pipelineLayout);
        static bool createFramebuffer(VkDevice device, const VkExtent2D& size, VkRenderPass renderPass, const Vector<VkImageView>& attachments, VkFramebuffer& framebuffer);
        static bool createCommandPool(VkDevice device, u32 familyQueueIndex, VkCommandPool& pool);
        static bool createVertexBuffer(VkDevice device, const VkPhysicalDeviceMemoryProperties& memProperties, const void* data, u32 bufferSize, bool shareInQueues, bool coherentMemory, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        static void createVertexAttribs(const struct SubmeshInput& sinput, Vector<VkVertexInputAttributeDescription>& inputAttribs, u32& vertexSize);

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

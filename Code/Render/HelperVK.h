#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include <vulkan/vulkan.h>

namespace fv
{
    struct SwapChainInfoVK
    {
        VkSurfaceCapabilitiesKHR capabilities;
        Vector<VkSurfaceFormatKHR> formats;
        Vector<VkPresentModeKHR> presentModes;
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
        VkPhysicalDeviceFeatures features;
        QueueFamilyIndicesVK queueIndices;
        VkPipeline opaquePipeline;
        VkPipelineLayout opaquePipelineLayout;
    };

    struct SwapChainVK
    {
        DeviceVK* device;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapChain;
        VkSwapchainKHR oldSwapChain;
        Vector<VkImage> images;
        Vector<VkImageView> imgViews;
    };

    struct SwapChainParamsVK
    {
        DeviceVK* device;
        VkSurfaceKHR surface;
        u32 width, height;
        u32 imageCount;
        u32 imageArrayLayerCount;
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
        static bool createSwapChain(const SwapChainParamsVK& params, SwapChainVK& swapChain);
        static bool createImageView(VkDevice device, VkImage image, VkFormat format, VkImageView& imgView);
        static bool createShaderFromBinary(VkDevice device, const Path& path, VkShaderModule& shaderModule);
        static bool createShaderModule(VkDevice device, const Vector<char>& code, VkShaderModule& shaderModule);
        static bool createBasePipeline(VkDevice device, VkShaderModule vertShader, VkShaderModule fragShader, VkRenderPass renderPass,
                                       VkExtent2D vpSize, VkPipelineLayout& pipelineLayout, VkPipeline& pipeline);

        // Checks and validation.
        static void queryRequiredWindowsExtensions(void* pWindow, Vector<const char*>& listToFill);
        static bool checkRequiredLayers(const Vector<const char*>& requiredList, VkPhysicalDevice physicalDevice=nullptr);
        static bool checkRequiredExtensions(const Vector<const char*>& requiredList, VkPhysicalDevice physicalDevice=nullptr);
        static void queryExtensions(Vector<String>& extensions, VkPhysicalDevice physicalDevice=nullptr);
        static void queryLayers(Vector<String>& layers, VkPhysicalDevice physicalDevice=nullptr);
        static bool validateNameList(const Vector<String>& found, const Vector<const char*>& required);

        // Swap chain query and pick.
        static void querySwapChainInfo(VkPhysicalDevice device, VkSurfaceKHR surface, SwapChainInfoVK& info);
        static bool chooseSwapChain(u32 width, u32 height, const SwapChainInfoVK& info, VkSurfaceFormatKHR& format, VkPresentModeKHR& mode, VkExtent2D& extend);

        // Compound helpers. Stores data directly in DeviceVK.
        static void storeDevicePropertiesAndFeatures(DeviceVK& device);
        static void storeDeviceQueueFamilies(DeviceVK& device, VkSurfaceKHR mainSurface);
    };
}
#endif

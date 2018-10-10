#pragma once
#include "RenderManager.h"
#if FV_VULKAN
#include "../Core/ObjectManager.h"
#include "GraphicResourceVK.h"
#include "vulkan/vulkan.h"

namespace fv
{
    class GraphicResource;

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


    class RenderManagerVK : public RenderManager
    {
    public:
        RenderManagerVK();
        ~RenderManagerVK() override;
        bool initGraphics() override;
        void closeGraphics() override;
        FV_TS GraphicResource* createGraphic(u32 resourceType, u32 deviceIdx=0) override;
        FV_TS void freeGraphic(GraphicResource* resource, bool async=false) override;

        // Debug callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

    private:
        void readRenderConfig(RenderConfig& renderSetup);
        bool createWindows(const RenderConfig& renderSetup);
        bool createIntance(const String& name);
        bool trySetupDebugCallback(bool includeVerbose, bool includeInfo);
        bool createDevices(VkSurfaceKHR mainSurface);
        bool createSurface(const void* wHandle, VkSurfaceKHR& surface);
        bool createSwapChain(const SwapChainParamsVK& params, SwapChainVK& swapChain);
        bool chooseSwapChain(u32 width, u32 height, const SwapChainInfoVK& info, VkSurfaceFormatKHR& format, VkPresentModeKHR& mode, VkExtent2D& extend);
        void storeDeviceProperties(DeviceVK& device);
        void storeDeviceQueueFamilies(DeviceVK& device, VkSurfaceKHR mainSurface);
        void querySwapChainInfo(VkPhysicalDevice device, VkSurfaceKHR surface, SwapChainInfoVK& info);
        bool checkRequiredLayers(const Vector<const char*>& requiredList, VkPhysicalDevice physicalDevice=nullptr);
        bool checkRequiredExtensions(const Vector<const char*>& requiredList, VkPhysicalDevice physicalDevice=nullptr);
        void queryExtensions(Vector<String>& extensions, VkPhysicalDevice physicalDevice=nullptr);
        void queryLayers(Vector<String>& layers, VkPhysicalDevice physicalDevice=nullptr);
        bool validateNameList(const Vector<String>& found, const Vector<const char*>& required);

        // 
        VkImageView createImageView(VkDevice device, VkImage image, VkFormat format);

        VkInstance m_Instance{};
        VkDebugUtilsMessengerEXT m_DebugCallback{};
        Vector<DeviceVK> m_Devices;
        Vector<const char*> m_RequiredInstanceExtensions;
        Vector<const char*> m_RequiredInstanceLayers;
        Vector<const char*> m_RequiredPhysicalExtensions;
        Vector<const char*> m_RequiredPhysicalLayers;
        SwapChainVK m_MainSwapChain{};
        void* m_MainWindow{};
        void* m_SecondaryWindow{};

        ObjectManager<GraphicResourceVK> m_Graphics;
    };
}
#endif
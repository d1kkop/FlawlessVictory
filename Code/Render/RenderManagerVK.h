#pragma once
#include "RenderManager.h"
#if FV_VULKAN
#include "HelperVK.h"
#include "GraphicResourceVK.h"
#include "../Core/ObjectManager.h"

namespace fv
{
    class GraphicResource;
    enum class GraphicType;


    class RenderManagerVK : public RenderManager
    {
    public:
        RenderManagerVK();
        ~RenderManagerVK() override;
        bool initGraphics() override;
        void closeGraphics() override;
        FV_TS GraphicResource* createGraphic(GraphicType type, u32 deviceIdx=0) override;
        FV_TS void freeGraphic(GraphicResource* resource, bool async=true) override;
        void drawFrame() override;
        void waitOnDeviceIdle() override;

        // Debug callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

    private:
        bool createDevices(VkSurfaceKHR mainSurface);
        bool createSwapChain(DeviceVK& device, SwapChainVK& swapChain, const RenderConfig& rc);
        bool createImagesFromSwapChain(const SwapChainVK& swapChain, u32 numLayers, Vector<VkImage>& images, Vector<VkImageView>& imageViews);
        bool createImagesForDevice(DeviceVK& dv, const RenderConfig& rc);
        bool createFrameBuffersForDevice(DeviceVK& device, VkRenderPass renderPass);
        bool createStandardShaders(DeviceVK& device);
        bool createFrameSyncObjects(DeviceVK& device, u32 numFramesBehind);

        VkInstance m_Instance{};
        VkDebugUtilsMessengerEXT m_DebugCallback{};
        Vector<DeviceVK> m_Devices;
        Vector<const char*> m_RequiredInstanceExtensions;
        Vector<const char*> m_RequiredInstanceLayers;
        Vector<const char*> m_RequiredPhysicalExtensions;
        Vector<const char*> m_RequiredPhysicalLayers;
        SwapChainVK m_MainSwapChain{};
        VkCommandBuffer m_DrawTriangle{};
        void* m_MainWindow{};
        void* m_SecondaryWindow{};
        u32 m_FrameImageIdx = 0; // Iterates from 0 to RenderConfig.numFramesBehind-1
        u32 m_CurrentDrawImage = 0; // In case of no swap chain.

        ObjectManager<GraphicResourceVK> m_Graphics;
    };
}
#endif
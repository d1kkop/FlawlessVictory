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
        FV_TS void freeGraphic(GraphicResource* resource, bool async=false) override;

        // Debug callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

    private:
        void readRenderConfig(RenderConfig& renderSetup);
        bool createDevices(VkSurfaceKHR mainSurface);
        bool createStandardShaders();

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
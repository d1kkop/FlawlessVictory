#pragma once
#include "RenderManager.h"
#if FV_VULKAN
#include "vulkan/vulkan.h"

namespace fv
{
    class GraphicResource;

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
        String name;
        VkDevice device;
        VkPhysicalDevice physicalDevice;
        VkQueue graphicsQueue;
        VkQueue computeQueue;
        VkQueue transferQueue;
        VkQueue sparseQueue;
        VkQueue presentQueue;
    };


    class RenderManagerVK : public RenderManager
    {
    public:
        ~RenderManagerVK() override;
        bool initGraphics(const RenderManagerParams& params) override;
        void closeGraphics() override;
        GraphicResource* createGraphic() override;
        void RenderManager::render(const class Camera* camera) override;

        // Debug callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

    private:
        void obtainExtensions();
        void obtainLayers();
        bool createIntance();
        bool trySetupDebugCallback();
        bool createDevices();
        bool createSurfaces();
        bool isDeviceSuitable(VkPhysicalDevice device, String& deviceNames, QueueFamilyIndicesVK& queueIndices);

        RenderManagerParams m_CreateParams{};
        VkInstance m_Instance{};
        VkDebugUtilsMessengerEXT m_DebugCallback{};
        Vector<DeviceVK> m_Devices;
        Vector<const char*> m_Extensions;
        Vector<const char*> m_Layers;
        VkSurfaceKHR m_WindowSurface{};
    };
}
#endif
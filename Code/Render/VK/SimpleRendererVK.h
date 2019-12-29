#pragma once
#include "../RenderManager.h"
#if FV_VULKAN
#include "IncVulkan.h"

namespace fv
{
    class VKInstance;

    class SimpleRendererVK : public RenderManager
    {
    public:
        SimpleRendererVK();
        ~SimpleRendererVK() override;
        FV_BG bool initGraphics() override;
        void closeGraphics() override;
        void render() override;


        // Debug callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

        bool createInstance();
        bool createDevice();
        bool createWindow();

        void destroyWindow();

        M<VKInstance> m_Instance;

        // TODO remove
        Vector<const char*> m_RequiredInstanceExtensions;
        Vector<const char*> m_RequiredInstanceLayers;
        Vector<const char*> m_RequiredPhysicalExtensions;
        Vector<const char*> m_RequiredPhysicalLayers;
    };
}
#endif
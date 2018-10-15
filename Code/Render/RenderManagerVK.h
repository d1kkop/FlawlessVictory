#pragma once
#include "RenderManager.h"
#if FV_VULKAN
#include "DeviceVK.h"
#include "SwapChainVK.h"

namespace fv
{
    class RenderManagerVK : public RenderManager
    {
    public:
        RenderManagerVK();
        ~RenderManagerVK() override;
        bool initGraphics() override;
        void closeGraphics() override;
        void drawFrame() override;
        void waitOnDeviceIdle() override;

        FV_TS bool getOrCreatePipeline(u32 deviceIdx, const SubmeshInput& sinput, const MaterialData& matData, VkRenderPass renderPass, PipelineVK& pipeline);

        // Debug callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

    private:
        bool createDevices(VkSurfaceKHR mainSurface);

        u64 createTexture2D(u32 width, u32 height, const char* data, u32 size, ImageFormat format) override;
        u64 createShader(const char* data, u32 size) override;
        u64 createSubmesh(const Submesh& submesh, const MaterialData& matData) override;
        void deleteTexture2D(u64 tex2d) override;
        void deleteShader(u64 shader) override;
        void deleteSubmesh(u64 submesh) override;

        VkInstance m_Instance{};
        VkDebugUtilsMessengerEXT m_DebugCallback{};
        Vector<DeviceVK*> m_Devices; // Must use ptr as has mutexes for which default operators are deleted.
        SwapChainVK m_MainSwapChain{};
        void* m_Window {};
        u32 m_FrameImageIdx = 0;    // Iterates from 0 to RenderConfig.numFramesBehind-1
        u32 m_CurrentDrawImage = 0; // In case of no swap chain.

        // TODO remove
        Vector<const char*> m_RequiredInstanceExtensions;
        Vector<const char*> m_RequiredInstanceLayers;
        Vector<const char*> m_RequiredPhysicalExtensions;
        Vector<const char*> m_RequiredPhysicalLayers;
    };
}
#endif
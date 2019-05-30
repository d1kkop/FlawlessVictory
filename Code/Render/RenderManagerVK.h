#pragma once
#include "RenderManager.h"
#if FV_VULKAN
#include "../Core/JobManager.h"
#include "DeviceVK.h"
#include "SwapChainVK.h"
#include "PipelineVK.h"

namespace fv
{
    class RenderManagerVK : public RenderManager
    {
    public:
        RenderManagerVK();
        ~RenderManagerVK() override;
        FV_BG bool initGraphics() override;
        void closeGraphics() override;
        void drawFrame(const Mat4& view) override;

        void waitOnDeviceIdle() override;

        // Debug callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

    private:
        bool createDevices(VkSurfaceKHR mainSurface, u32 numGraphicsQueues);
        u32 numDevices() const override { return (u32) m_Devices.size(); }
        FV_TS u32 autoDeviceIdx() override;

        // Create
        FV_TS RTexture2D createTexture2D(u32 deviceIdx, u32 width, u32 height, const char* data, u32 size,
                                         u32 mipLevels, u32 layers, u32 samples, ImageFormat format) override;
        FV_TS RShader createShader(u32 deviceIdx, const char* data, u32 size) override;
        FV_TS RSubmesh createSubmesh(u32 deviceIdx, const Submesh& submesh, const SubmeshInput& si) override;

        // Delete
        FV_TS void deleteTexture2D(RTexture2D tex2d) override;
        FV_TS void deleteShader(RShader shader) override;
        FV_TS void deleteSubmesh(RSubmesh submesh) override;

        // Rendering
        void addToRenderList(u32 tIdx, RSubmesh submesh, const MaterialData& mdata, DrawMethod drawMethod) override;

        VkInstance m_Instance{};
        VkSurfaceKHR m_Surface{};
        VkDebugUtilsMessengerEXT m_DebugCallback{};
        Vector<DeviceVK*> m_Devices; // Must use ptr as has mutexes for which default operators are deleted.
        void* m_Window {};
        u32 m_FrameIndex = 0;    // Iterates from 0 to RenderConfig.numFramesBehind-1
        u32 m_CurrentDrawImage = 0; // In case of no swap chain.
        Atomic<u32> m_AutoDeviceIdx;

        // TODO remove
        Vector<const char*> m_RequiredInstanceExtensions;
        Vector<const char*> m_RequiredInstanceLayers;
        Vector<const char*> m_RequiredPhysicalExtensions;
        Vector<const char*> m_RequiredPhysicalLayers;

        PipelineFormatVK m_PipelineFormatOpaque{};
        PipelineFormatVK m_PipelineFormatTransparent{};
        Vector<M<Job>> m_RenderJobs;
    };
}
#endif
#pragma once
#include "../RenderManager.h"
#if FV_VULKAN
#include "IncVulkan.h"

namespace fv
{
    class InstanceVK;
    class DeviceVK;
    class SurfaceVK;
    class SwapChainVK;
    class AllocatorVK;
    class RenderPassVK;
    class PipelineLayoutVK;
    class PipelineVK;
    class ShaderVK;
    class FrameBufferVK;

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

        bool createWindow();
        bool createInstance();
        bool createSurface();
        bool createDevice();
        bool createSwapChain();
        bool createAllocator();
        bool createShaders();
        bool createSimplePass();
        bool createPipelineLayout();
        bool createPipeline();
        bool createFramebuffer();

        void destroyWindow();

        M<InstanceVK>  m_Instance;
        M<DeviceVK>    m_Device;
        M<SurfaceVK>   m_Surface;
        M<SwapChainVK> m_SwapChain;
        M<AllocatorVK> m_Allocator;

        // Simple rendering
        M<ShaderVK> m_VertexShaderSimple;
        M<ShaderVK> m_FragmentShaderSimple;
        M<RenderPassVK> m_SimplePlass;
        M<PipelineLayoutVK> m_EmptyPipelineLayout;
        M<PipelineVK> m_SimplePipeline;
        List<M<FrameBufferVK>> m_FrameBuffers;

        // TODO remove
        Vector<const char*> m_RequiredInstanceExtensions;
        Vector<const char*> m_RequiredInstanceLayers;
        Vector<const char*> m_RequiredPhysicalExtensions;
        Vector<const char*> m_RequiredPhysicalLayers;
    };
}
#endif
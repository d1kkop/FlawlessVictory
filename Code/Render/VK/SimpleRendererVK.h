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
    class CommandBuffersVK;
    class CommandPoolVK;
    class SemaphoreVK;
    class DeviceResoure;

    class SimpleRendererVK : public RenderManager
    {
    public:
        SimpleRendererVK();
        ~SimpleRendererVK() override;
        FV_BG bool initGraphics() override;
        void closeGraphics() override;

        M<DeviceResource> createShader( const char* code, u32 size ) override;

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
        bool createCommandPool();
        bool createShaders();
        bool createSimplePass();
        bool createPipelineLayout();
        bool createPipeline();
        bool createFramebuffers();
        bool createTriangleCommanBuffer();
        bool createSemaphores();

        void render() override;

        void destroyWindow();

        M<InstanceVK>  m_Instance;
        M<DeviceVK>    m_Device;
        M<SurfaceVK>   m_Surface;
        M<SwapChainVK> m_SwapChain;
        M<AllocatorVK> m_Allocator;
        M<CommandPoolVK> m_CommandPool;

        // Simple rendering
        M<ShaderVK> m_VertexShaderSimple;
        M<ShaderVK> m_FragmentShaderSimple;
        M<RenderPassVK> m_SimplePlass;
        M<PipelineLayoutVK> m_EmptyPipelineLayout;
        M<PipelineVK> m_SimplePipeline;
        M<CommandBuffersVK> m_TriangleCommandBuffer;
        List<M<FrameBufferVK>> m_FrameBuffers;
        List<M<SemaphoreVK>> m_FrameImageAvailableSemaphore;
        List<M<SemaphoreVK>> m_FrameTriangleDoneSemaphore;
        u32 m_FrameObjectIdx = 0;

        // TODO remove
        Vector<const char*> m_RequiredInstanceExtensions;
        Vector<const char*> m_RequiredInstanceLayers;
        Vector<const char*> m_RequiredPhysicalExtensions;
        Vector<const char*> m_RequiredPhysicalLayers;
    };
}
#endif
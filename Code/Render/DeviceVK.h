#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "PCH.h"
#include "RenderImageVK.h"
#include "FrameSyncObjectVK.h"
#include "PipelineVK.h"
#include "RenderManager.h"

namespace fv
{
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
    public:
        DeviceVK() = default;
        void release(); // Do not put in destructor.
        void storeDeviceQueueFamilies(VkSurfaceKHR surface);
        void setFormatAndExtent(const struct RenderConfig& rc);
        bool createCommandPools();
        bool createSwapChain(const struct RenderConfig& rc, VkSurfaceKHR surface);
        bool createStandard(const struct RenderConfig& rc);
        bool createRenderImages(const struct RenderConfig& rc);
        bool createFrameSyncObjects(const struct RenderConfig& rc);
        bool getOrCreatePipeline(const struct SubmeshInput& sinput, const struct MaterialData& matData, VkRenderPass renderPass, PipelineVK& pipelineOut);
        bool recordCommandBuffer(const Function<void (VkCommandBuffer, const RenderImageVK&)>& recordCb);

        FV_TS RTexture2D createTexture2D(u32 width, u32 height, const char* data, u32 size, ImageFormat format);
        FV_TS RShader createShader(const char* data, u32 size);
        FV_TS RSubmesh createSubmesh(const Submesh& submesh);
        FV_TS void deleteTexture2D(RTexture2D tex2d);
        FV_TS void deleteShader(RShader shader);
        FV_TS void deleteSubmesh(RSubmesh submesh);

        u32 idx;
        VkInstance instance;
        VkDevice logical;
        VkPhysicalDevice physical;
        VkQueue graphicsQueue;
        VkQueue computeQueue;
        VkQueue transferQueue;
        VkQueue sparseQueue;
        VkQueue presentQueue;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memProperties;
        VkPhysicalDeviceFeatures features;
        QueueFamilyIndicesVK queueIndices;
        VkCommandPool commandPool;
        VkExtent2D extent;
        VkFormat format;
        VkShaderModule standardFrag;
        VkShaderModule standardVert;
        VkRenderPass clearPass;
        PipelineVK clearPipeline;
        Map<u32, PipelineVK> pipelines;
        Vector<RenderImageVK> renderImages;
        Vector<FrameSyncObjectVK> frameSyncObjects;
        Vector<DeviceResource> textures2d;
        Vector<DeviceResource> shaders;
        Vector<DeviceResource> submeshes;
        struct SwapChainVK* swapChain;

    private:
        Mutex pipelineMutex;
        Mutex tex2dMutex;
        Mutex shaderMutex;
        Mutex submeshMutex;
    };
}
#endif
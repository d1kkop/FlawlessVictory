#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "PCH.h"
#include "RenderImageVK.h"
#include "FrameSyncObjectVK.h"
#include "PipelineVK.h"

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
        bool prepareNewFrame(u32 waitFrameIdx, RenderImageVK& newRenderImage);

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
        Map<u32, PipelineVK> pipelines;
        Vector<VkImage> textures2d;
        Vector<VkShaderModule> shaders;
        Vector<VkBuffer> buffers;
        Vector<VkDeviceMemory> deviceMemorys;
        Vector<RenderImageVK> renderImages;
        Vector<FrameSyncObjectVK> frameSyncObjects;
        struct SwapChainVK* swapChain;

    private:
        Mutex pipelineMutex;
    };
}
#endif
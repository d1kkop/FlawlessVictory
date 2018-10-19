#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
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
        bool createAllocators();
        bool createCommandPools();
        bool createStagingBuffers();
        bool createSwapChain(const struct RenderConfig& rc, VkSurfaceKHR surface);
        bool createStandard(const struct RenderConfig& rc);
        bool createRenderImages(const struct RenderConfig& rc);
        bool createFrameSyncObjects(const struct RenderConfig& rc);
        void addFrameCmd(const Function<void (VkCommandBuffer, const RenderImageVK&)>& recordCb);
        FV_TS VkCommandBuffer addSingleTimeCmd(const Function<void (VkCommandBuffer)>& recordCb);
        FV_TS void addSingleTimeCmd2(const Function<void (VkCommandBuffer)>& recordCb);
        FV_TS bool getOrCreatePipeline(const struct SubmeshInput& sinput, const struct MaterialData& matData, VkRenderPass renderPass, PipelineVK& pipelineOut);

        FV_TS RTexture2D createTexture2D(u32 width, u32 height, const char* data, u32 size, u32 mipLevels, u32 layers, u32 samples,
                                         ImageFormat format, VkImageUsageFlagBits imageUsageBits, VmaMemoryUsage memoryUsage);
        FV_TS RShader createShader(const char* data, u32 size);
        FV_TS RSubmesh createSubmesh(const Submesh& submesh, const SubmeshInput& si);
        FV_TS void deleteTexture2D(RTexture2D tex2d, bool removeFromList);
        FV_TS void deleteShader(RShader shader, bool removeFromList);
        FV_TS void deleteSubmesh(RSubmesh submesh, bool removeFromList);

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
        VkCommandPool graphicsPool;
        VkCommandPool transferPool;
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
        Vector<VkCommandBuffer> singleTimeCmds;
        struct SwapChainVK* swapChain;
        struct BufferVK* stagingBuffer;
        void* stagingMemory;
        VmaAllocator allocator;
        Mutex singleTimeCmdsMutex;

    private:
        Mutex pipelineMutex;
        Mutex tex2dMutex;
        Mutex shaderMutex;
        Mutex submeshMutex;
    };
}
#endif
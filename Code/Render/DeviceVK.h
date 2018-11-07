#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "RenderImageVK.h"
#include "PipelineVK.h"
#include "BufferVK.h"
#include "RenderPassVK.h"
#include "RenderManager.h"

namespace fv
{
    struct SubmeshVK;
    struct PipelineVK;

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
        void releaseSwapchain();
        void storeDeviceQueueFamilies(VkSurfaceKHR surface);
        void setFormatAndExtent(const RenderConfig& rc);
        bool createAllocators();
        bool createCommandPools();
        bool createStagingBuffers();
        bool createSwapChain(const RenderConfig& rc, VkSurfaceKHR surface);
        bool createRenderPasses(const RenderConfig& rc);
        bool createRenderImages(const RenderConfig& rc);
        bool createFrameObjects(const RenderConfig& rc);
        bool createPipelines(const RenderConfig& rc);
        bool createStandard(const RenderConfig& rc);
        bool reCreateSwapChain(const RenderConfig& rc, VkSurfaceKHR surface);

        void submitGraphicsCommands(u32 frameIndex, u32 queueIdx, VkSemaphore waitSemaphore, const Function<bool (VkCommandBuffer cb)>& callback);
        void submitOnetimeTransferCommand(const Function<void (VkCommandBuffer)>& callback);

        // Resource creation/deletion
        FV_TS RTexture2D createTexture2D(u32 width, u32 height, const char* data, u32 size, u32 mipLevels, u32 layers, u32 samples, ImageFormat format, VkImageUsageFlagBits imageUsageBits, VmaMemoryUsage memoryUsage);
        FV_TS RShader createShader(const char* data, u32 size);
        FV_TS RSubmesh createSubmesh(const Submesh& submesh, const SubmeshInput& si);
        FV_TS void deleteTexture2D(RTexture2D tex2d, bool removeFromList);
        FV_TS void deleteShader(RShader shader, bool removeFromList);
        FV_TS void deleteSubmesh(RSubmesh submesh, bool removeFromList);
        FV_TS PipelineVK* getOrCreatePipeline(u32 pipelineHash, const struct PipelineFormatVK& format);
        FV_TS PipelineVK* getOrCreatePipeline(const struct PipelineFormatVK& format);

        // Buffers
        FV_TS bool mapStagingBuffer(BufferVK& staging, void** pMapped);
        FV_TS void unmapStagingBuffer();

        // Frame syncronization and submission
        void waitForFences(u32 frameIndex);
        void resetFences(u32 frameIndex);

        u32 idx;
        VmaAllocator allocator;
        VkInstance instance;
        VkDevice logical;
        VkPhysicalDevice physical;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memProperties;
        VkPhysicalDeviceFeatures features;
        QueueFamilyIndicesVK queueIndices;
        VkQueue computeQueue;
        VkQueue sparseQueue;
        VkQueue presentQueue;
        VkExtent2D extent;
        VkFormat format;
        
        // Transfer
        VkQueue transferQueue;
        VkCommandPool transferPool;
        Vector<VkCommandBuffer> singleTimeCmds;

        // Resources
        Vector<DeviceResource> textures2d;
        Vector<DeviceResource> shaders;
        Vector<RSubmesh> submeshes;

        // Standard
        VkShaderModule standardFrag;
        VkShaderModule standardVert;
        RenderPassVK clearColorDepthPass;
        RenderPassVK renderStandardPass;

        // Per frame per thread, eg 2 frames 4 threads is array size of 8.
        Vector<VkSemaphore> frameFinishSemaphores;
        Vector<Vector<VkCommandBuffer>> frameGraphicsCmds;

        // Per thread
        Vector<VkCommandBuffer> activeGraphicsCmdBuffer;
        Vector<VkCommandPool> graphicsPools;
        Vector<VkQueue> graphicsQueues;
        Vector<Map<PipelineVK*, Vector<SubmeshVK*>>> renderLists;

        // Per frame
        Vector<RenderImageVK> renderImages;
        Vector<VkFence> frameFinishFences;

        // Pipelines
        Map<u32, PipelineVK> pipelines;
        //PipelineVK pipelineOpaqueStandard;
        //PipelineVK pipelineOpaqueStandardTB;
        //PipelineVK pipelineOpaqueStandardTBBones;
        //PipelineVK pipelineOpaqueStandardBones;

        // Swapchain
        struct SwapChainVK* swapChain;
        bool recreateSwapChain;

    private:
        Mutex stageBufferMtx;
        BufferVK stageBuffer;
        Mutex pipelineMutex;
        Mutex tex2dMutex;
        Mutex shaderMutex;
        Mutex submeshMutex;

        friend class RenderManagerVK;
    };
}
#endif
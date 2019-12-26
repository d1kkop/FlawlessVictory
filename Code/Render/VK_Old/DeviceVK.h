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
        static DeviceVK* create(VkInstance instance, VkPhysicalDevice physical, u32 idx, VkSurfaceKHR surface, u32 numGraphicQueues,
                                const Vector<const char*>& physicalExtensions,
                                const Vector<const char*>& physicalLayers);
        void release(); // Do not put in destructor.
        void releaseSwapchain();

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

        void submitGraphicsCommand( u32 frameIndex, u32 queueIdx,
                                    VkCommandBuffer commandBuffer,
                                    VkSemaphore waitSemaphore, VkSemaphore signalSemaphore,
                                    bool submit,
                                    const Function<bool( VkCommandBuffer cb )>& callback );

        void submitOnetimeTransferCommand(const Function<void (VkCommandBuffer)>& callback);

        // Resource creation/deletion
        FV_TS RTexture2D createTexture2D(u32 width, u32 height, const char* data, u32 size, u32 mipLevels, u32 layers, u32 samples, ImageFormat format, VkImageUsageFlagBits imageUsageBits, VmaMemoryUsage memoryUsage);
        FV_TS RTexture2D createTexture2D(u32 width, u32 height, const char* data, u32 size, u32 mipLevels, u32 layers, u32 samples, VkFormat format, VkImageUsageFlagBits imageUsageBits, VmaMemoryUsage memoryUsage);
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
        void waitForFrameFinishFences(u32 frameIndex);
        void resetFrameFinishFences(u32 frameIndex);

        // Rendering
        void prepareDrawMethods( u32 frameIdx, u32 tIdx );
        void renderDrawMethod( DrawMethod drawMethod, VkFramebuffer fb, u32 frameIdx, u32 tIdx, VkSemaphore waitSemaphore );

        u32 idx = -1;
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
        Vector<RTexture2D> textures2d;
        Vector<DeviceResource> shaders;
        Vector<RSubmesh> submeshes;

        // Standard
        VkShaderModule standardFrag;
        VkShaderModule standardVert;
        RenderPassVK clearColorDepthPass;
        Vector<RenderPassVK> drawMethods;

        // Per thread, for K frames behind
        Vector<Vector<VkCommandBuffer>> frameGraphicsCmds;
        Vector<Vector<VkSemaphore>> frameFinishSemaphores;
        Vector<Vector<VkFence>> frameFinishFences;

        // Per thread
        Vector<VkCommandPool> graphicsPools;
        Vector<VkQueue> graphicsQueues;

        // Per Thread per DrawMethod per Pipeline a list of submeshes.
        Vector<Vector<HashMap<PipelineVK*, Vector<SubmeshVK*>>>> renderListsMt;

        // Per frame
        Vector<RenderImageVK> renderImages;

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
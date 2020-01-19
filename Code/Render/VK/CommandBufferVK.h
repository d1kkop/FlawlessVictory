#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    enum class CommandBufferUsage
    {
        OneTime,
        EntirelyInRenderPass,
        CanResubmit
    };

    class CommandPoolVK;
    class RenderPassVK;
    class FrameBufferVK;
    class PipelineVK;
    class BufferVK;

    class CommandBufferVK
    {
    public:
        using VkType = VkCommandBuffer;

    public:
        CommandBufferVK() = default;
        ~CommandBufferVK();

    public:
        static M<CommandBufferVK> allocate( const M<CommandPoolVK>& fromPool, bool fromSecondary=false );

        VkCommandBuffer vk() const { return m_CommandBuffer; }
        const VkCommandBuffer* vkp() const { return &m_CommandBuffer; }
        void resetBufferState();

        void beginRecord( CommandBufferUsage usage );
        void beginRenderPass( const M<RenderPassVK>& renderPass, const M<FrameBufferVK>& frameBuffer,
                              const Vec4& clearColor ={ .5f, .5f, 1.f, 1 }, const VkOffset2D& offset ={ 0, 0 }, const VkExtent2D& extent ={ (uint32_t)-1, (uint32_t)-1 } );
        void bindPipeline( const M<PipelineVK>& pipeline );
        void bindVertices( const M<BufferVK>& vertexBuffer );
        void draw( u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0 );
        void endRenderPass();
        void endRecord();

        const M<CommandPoolVK>& commandPool() const { return m_CommandPool; }

    private:
        VkCommandBuffer m_CommandBuffer;
        M<CommandPoolVK> m_CommandPool;
    };
}
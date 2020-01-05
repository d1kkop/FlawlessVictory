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

    class CommandBuffersVK
    {
    public:
        CommandBuffersVK() = default;
        ~CommandBuffersVK();

    public:
        static M<CommandBuffersVK> allocate( const M<CommandPoolVK>& fromPool, u32 numBuffers, bool fromSecondary=false );

        u32 num() const { return (u32)m_CommandBuffers.size(); }
        const VkCommandBuffer* getAll() const { return m_CommandBuffers.data(); }

        void resetBufferState( u32 idx=0 );
        void resetEntireBufferState();

        void begin( CommandBufferUsage usage, u32 idx=0 );
        VkCommandBuffer vk( u32 idx=0 ) const { return m_CommandBuffers[idx]; }
        void end( u32 idx=0 );

        const M<CommandPoolVK>& commandPool() const { return m_CommandPool; }

    private:
        List<VkCommandBuffer> m_CommandBuffers;
        M<CommandPoolVK> m_CommandPool;
    };
}
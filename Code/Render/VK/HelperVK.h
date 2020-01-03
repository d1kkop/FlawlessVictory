#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class SwapChainVK;

    class VertexDescriptorSetVK
    {
    public:
        void addBinding( u32 binding, u32 vertexSize );
        void addAttrib( u32 binding, u32 location, VkFormat format, u32 offset );

        u32 numInputBindings() const { return (u32)m_VertexInputs.size(); }
        const List<VkVertexInputBindingDescription>& inputBindings() const { return m_VertexInputs; }

        u32 numInputAttribs() const { return (u32)m_AttribInputs.size(); }
        const List<VkVertexInputAttributeDescription>& inputAttribs() const { return m_AttribInputs; }

    private:
        List<VkVertexInputBindingDescription> m_VertexInputs;
        List<VkVertexInputAttributeDescription> m_AttribInputs;
    };


    class HelperVK
    {
    public:
        template <typename From, typename To> 
        static void toVkList(const List<From>& from, List<To>& to)
        {
            for ( auto& f : from ) 
                to.emplace_back( f->vk() );
        }

        static VkViewport makeSimpleViewport( const M<SwapChainVK>& swapChain );
    };
}
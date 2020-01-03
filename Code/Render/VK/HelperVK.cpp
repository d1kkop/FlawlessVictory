#include "HelperVK.h"
#include "SwapChainVK.h"

namespace fv
{
    // ------- VertexDescriptorSet -------------------------------------------------------------------

    void VertexDescriptorSetVK::addBinding( u32 binding, u32 vertexSize )
    {
        VkVertexInputBindingDescription input;
        input.binding = binding;
        input.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        input.stride = vertexSize;
        m_VertexInputs.emplace_back( input );
    }

    void VertexDescriptorSetVK::addAttrib( u32 binding, u32 location, VkFormat format, u32 offset )
    {
        VkVertexInputAttributeDescription input;
        input.binding = binding;
        input.format = format;
        input.offset = offset;
        input.location = location;
        m_AttribInputs.emplace_back( input );
    }

    // ------- HelperVK -------------------------------------------------------------------

    VkViewport HelperVK::makeSimpleViewport( const M<SwapChainVK>& swapChain )
    {
        VkViewport vp;
        vp.x = 0;
        vp.y = 0;
        vp.width  = (float) swapChain->extent().width;
        vp.height = (float) swapChain->extent().height;
        vp.minDepth = 0;
        vp.maxDepth = 1;
        return vp;
    }

}
#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    enum class ShaderTypeVK
    {
        Vertex,
        Fragment,
        Geometry,
        TesselationControl,
        TesselationEvaluation,
        Compute
    };

    enum class PrimitiveTypeVK
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        TriangleFan
    };

    enum class CullModeVK
    {
        None,
        Front,
        Back
    };

    enum class PolygonModeVK
    {
        Fill,
        Line,
        Point
    };

    enum class FrontFaceVK
    {
        CCW,
        CW
    };

    class RenderPassVK;
    class PipelineLayoutVK;
    class ShaderVK;

    class PipelineVK
    {
    public:
        PipelineVK() = default;
        ~PipelineVK();

    public:
        static M<PipelineVK> create( const M<RenderPassVK>& renderPass,
                                     const M<PipelineLayoutVK>& pipelineLayout,
                                     PrimitiveTypeVK primType,
                                     const M<ShaderVK>& vertexShader,
                                     const M<ShaderVK>& fragmentShader,
                                     const M<ShaderVK>& geometryShader,
                                     const M<ShaderVK>& tesselationControlShader,
                                     const M<ShaderVK>& tesselationEvaluationShader,
                                     const class VertexDescriptorSetVK& vertexDescriptorSet,
                                     const VkViewport& vp,
                                     bool depthTest = true,
                                     bool depthWrite = true,
                                     bool stencilTest = false,
                                     CullModeVK cullmode = CullModeVK::Back,
                                     PolygonModeVK polyMode = PolygonModeVK::Fill,
                                     FrontFaceVK frontFace = FrontFaceVK::CCW,
                                     float lineWidth = 1,
                                     u32 numSamples = 1 );

        VkPipeline vk() const { return m_Pipeline; }

    private:
        VkPipeline  m_Pipeline {};
        M<RenderPassVK> m_RenderPass;
        M<PipelineLayoutVK> m_PipelineLayout;
    };
}
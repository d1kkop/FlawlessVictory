#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    enum class ShaderType
    {
        Vertex,
        Fragment,
        Geometry,
        TesselationControl,
        TesselationEvaluation,
        Compute
    };

    enum class PrimitiveType
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        TriangleFan
    };

    class DeviceVK;

    class PipelineVK
    {
    public:
        PipelineVK() = default;
        ~PipelineVK();

    public:
        static M<PipelineVK> create( const M<DeviceVK>& device, 
                                     const M<RenderPassVK>& renderPass,
                                     const M<PipelineLayout>& pipelineLayout,
                                     PrimitiveType primType,
                                     const M<Shader>& vertexShader,
                                     const M<Shader>& fragmentShader,
                                     const M<Shader>& geometryShader,
                                     const M<Shader>& tesselationControlShader,
                                     const M<Shader>& tesselationEvaluationShader, 
                                     const Vector<VkVertexInputBindingDescription>& vertexBindings,
                                     const Vector<VkVertexInputAttributeDescription>& vertexAttribs,
                                     const VkViewport& vp,
                                     u32 vertexSize,
                                     bool depthTest = true,
                                     bool depthWrite = true,
                                     bool stencilTest = false,
                                     VkCullModeFlagBits cullmode,
                                     VkPolygonMode polyMode,
                                     VkFrontFace frontFace,
                                     float lineWidth = 1,
                                     u32 numSamples = 1 );

        VkShaderModule vk() const { return m_Pipeline; }

    private:
        VkPipeline  m_Pipeline {};
        M<DeviceVK> m_Device;
    };
}
#include "PipelineVK.h"
#include "DeviceVK.h"
#include "RenderPassVK.h"
#include "PipelineLayoutVK.h"
#include "ShaderVK.h"
#include "HelperVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    PipelineVK::~PipelineVK()
    {
        if ( m_Pipeline )
        {
            vkDestroyPipeline( m_RenderPass->device()->logical(), m_Pipeline, NULL );
        }
    }

    M<PipelineVK> PipelineVK::create( const M<RenderPassVK>& renderPass,
                                      const M<PipelineLayoutVK>& pipelineLayout,
                                      PrimitiveTypeVK primType,
                                      const M<ShaderVK>& vertexShader,
                                      const M<ShaderVK>& fragmentShader,
                                      const M<ShaderVK>& geometryShader,
                                      const M<ShaderVK>& tesselationControlShader,
                                      const M<ShaderVK>& tesselationEvaluationShader,
                                      const class VertexDescriptorSetVK& vertexDescriptorSet,
                                      const VkViewport& vp,
                                      bool depthTest,
                                      bool depthWrite,
                                      bool stencilTest,
                                      CullModeVK cullmode,
                                      PolygonModeVK polyMode,
                                      FrontFaceVK frontFace,
                                      float lineWidth,
                                      u32 numSamples )
    {
        List<VkPipelineShaderStageCreateInfo> shaderCreateInfos;

        VkPipelineShaderStageCreateInfo shaderCreateInfo ={};
        shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderCreateInfo.module = vertexShader->vk();
        shaderCreateInfo.pName = "main";
        shaderCreateInfos.emplace_back( shaderCreateInfo );

        shaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderCreateInfo.module = fragmentShader->vk();
        shaderCreateInfos.emplace_back( shaderCreateInfo );

        if ( geometryShader )
        {
            shaderCreateInfo.stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
            shaderCreateInfo.module = geometryShader->vk();
            shaderCreateInfos.emplace_back( shaderCreateInfo );
        }

        if ( tesselationControlShader )
        {
            shaderCreateInfo.stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            shaderCreateInfo.module = tesselationControlShader->vk();
            shaderCreateInfos.emplace_back( shaderCreateInfo );
        }

        if ( tesselationEvaluationShader )
        {
            shaderCreateInfo.stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            shaderCreateInfo.module = tesselationEvaluationShader->vk();
            shaderCreateInfos.emplace_back( shaderCreateInfo );
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo ={};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = vertexDescriptorSet.numInputBindings();
        vertexInputInfo.pVertexBindingDescriptions = vertexDescriptorSet.inputBindings().data();
        vertexInputInfo.vertexAttributeDescriptionCount = vertexDescriptorSet.numInputAttribs();
        vertexInputInfo.pVertexAttributeDescriptions = vertexDescriptorSet.inputAttribs().data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly ={};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = (VkPrimitiveTopology)primType;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkRect2D scissor;
        scissor.offset ={ 0, 0 };
        scissor.extent ={ (u32)vp.width, (u32)vp.height };

        VkPipelineViewportStateCreateInfo viewportState ={};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &vp;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer ={};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = (VkPolygonMode) polyMode;
        rasterizer.lineWidth = lineWidth;
        rasterizer.cullMode  = (VkCullModeFlags) cullmode;
        rasterizer.frontFace = (VkFrontFace) frontFace;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencil ={};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = depthTest;
        depthStencil.depthWriteEnable = depthWrite;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = stencilTest;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling ={};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable  = VK_FALSE; // TODO what is this?
        multisampling.rasterizationSamples = (VkSampleCountFlagBits)numSamples;

        VkPipelineColorBlendAttachmentState colorBlendAttachment ={};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending ={};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkGraphicsPipelineCreateInfo pipelineInfo ={};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = (u32)shaderCreateInfos.size();
        pipelineInfo.pStages = shaderCreateInfos.data(); // vertex and fragment (minimum)
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayout->vk();
        pipelineInfo.renderPass = renderPass->vk();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        VkPipeline pipeline;
        if ( vkCreateGraphicsPipelines( renderPass->device()->logical(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline ) != VK_SUCCESS )
        {
            LOGC( "VK Failed to create graphics pipeline." );
            return false;
        }

        M<PipelineVK> pipelineVK = std::make_shared<PipelineVK>();
        pipelineVK->m_Pipeline = pipeline;
        pipelineVK->m_RenderPass = renderPass;
        pipelineVK->m_PipelineLayout = pipelineLayout;
        return pipelineVK;
    }

}
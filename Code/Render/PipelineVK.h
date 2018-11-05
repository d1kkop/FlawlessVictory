#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "IncVulkan.h"
#include "RenderManagerVK.h"

namespace fv
{
    struct DeviceVK;

    struct PipelineFormatVK
    {
        SubmeshInput sinput;
        MaterialData mdata;
        VkRenderPass renderPass;
        VkCullModeFlagBits cullmode;
        VkPolygonMode polyMode;
        VkFrontFace frontFace;
        float lineWidth;
        u32 numSamples;
        u32 vertexSize;
    };

    struct PipelineVK
    {
    public:
        PipelineVK() = default;
        void release(); // Do not put in destructor.
        static PipelineVK create(DeviceVK& device, u32 pipelineHash,
                                 const PipelineFormatVK& format, const VkViewport& vp,
                                 const Vector<VkVertexInputBindingDescription>& vertexBindings,
                                 const Vector<VkVertexInputAttributeDescription>& vertexAttribs);
;

        bool valid() const { return m_Valid; }
        struct DeviceVK& device() const { return *m_Device; }
        VkPipeline pipeline() const { return m_Pipeline; }
        VkPipelineLayout layout() const { return m_Layout; }
        u32 hash() const { return m_Hash; }
        const PipelineFormatVK& format() const { return m_Format; }

        void bind(VkCommandBuffer cb);

    private:
        bool m_Valid = false;
        struct DeviceVK* m_Device;
        VkPipeline m_Pipeline;
        VkPipelineLayout m_Layout;
        PipelineFormatVK m_Format;
        u32 m_Hash;
    };
}
#endif
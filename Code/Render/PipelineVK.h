#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "IncVulkan.h"

namespace fv
{
    struct DeviceVK;

    struct PipelineVK
    {
    public:
        PipelineVK() = default;
        void release(); // Do not put in destructor.
        static PipelineVK create(DeviceVK& device, VkShaderModule vertShader, VkShaderModule fragShader, VkShaderModule geomShader,
                                 VkRenderPass renderPass, const VkViewport& vp, u32 vertexSize,
                                 const Vector<VkVertexInputBindingDescription>& vertexBindings,
                                 const Vector<VkVertexInputAttributeDescription>& vertexAttribs);

        bool valid() const { return m_Valid; }
        struct DeviceVK& device() const { return *m_Device; }
        VkPipeline pipeline() const { return m_Pipeline; }
        VkPipelineLayout layout() const { return m_Layout; }

        void recordCommandBuffers();
        void addBindPipelineToQueue( struct FrameObject& fo, u32 queueIdx );

    private:
        void recordBindBuffers(VkCommandBuffer cb);

        bool m_Valid = false;
        struct DeviceVK* m_Device;
        VkPipeline m_Pipeline;
        VkPipelineLayout m_Layout;
        Vector<VkCommandBuffer> m_BindBuffers;
    };
}
#endif
#include "PipelineVK.h"
#if FV_VULKAN
#include "DeviceVK.h"

namespace fv
{
    void PipelineVK::release()
    {
        if (!m_Device ||!m_Device->logical) return;
        vkDestroyPipeline( m_Device->logical, m_Pipeline, nullptr );
        vkDestroyPipelineLayout( m_Device->logical, m_Layout, nullptr );
    }

}
#endif
#include "PipelineLayoutVK.h"
#include "DeviceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    PipelineLayoutVK::~PipelineLayoutVK()
    {
        if ( m_PipelineLayout )
        {
            vkDestroyPipelineLayout( m_Device->logical(), m_PipelineLayout, NULL );
        }
    }

    M<PipelineLayoutVK> PipelineLayoutVK::create( const M<DeviceVK>& device )
    {
        VkPipelineLayoutCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.pNext = NULL;
        createInfo.setLayoutCount = 0;
        createInfo.pSetLayouts = NULL;
        createInfo.pushConstantRangeCount = 0;
        createInfo.pPushConstantRanges = NULL;

        VkPipelineLayout layout;
        if ( vkCreatePipelineLayout( device->logical(), &createInfo, NULL, &layout ) != VK_SUCCESS )
        {
            LOGC( "VK Failed to create pipelinelayout." );
            return {};
        }

        M<PipelineLayoutVK> pipelineLayoutVK = std::make_shared<PipelineLayoutVK>();
        pipelineLayoutVK->m_PipelineLayout = layout;
        pipelineLayoutVK->m_Device = device;
        return pipelineLayoutVK;
    }
}
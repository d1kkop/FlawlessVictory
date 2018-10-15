#include "PCH.h"
#if FV_VULKAN
#include "PipelineVK.h"
#include "DeviceVK.h"

namespace fv
{
    void PipelineVK::release()
    {
        vkDestroyPipeline( device->logical, pipeline, nullptr );
        vkDestroyPipelineLayout( device->logical, layout, nullptr );
    }

}
#endif
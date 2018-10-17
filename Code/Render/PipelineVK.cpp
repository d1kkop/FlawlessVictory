#include "PipelineVK.h"
#if FV_VULKAN
#include "DeviceVK.h"

namespace fv
{
    void PipelineVK::release()
    {
        if (!device ||!device->logical) return;
        vkDestroyPipeline( device->logical, pipeline, nullptr );
        vkDestroyPipelineLayout( device->logical, layout, nullptr );
    }

}
#endif
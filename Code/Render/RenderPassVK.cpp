#include "RenderPassVK.h"
#if FV_VULKAN
#include "DeviceVK.h"
#include "HelperVK.h"
#include "../Core/JobManager.h"

namespace fv
{
    void RenderPassVK::release()
    {
        if (!m_Device || !m_Device->logical) return;
        vkDestroyRenderPass( device().logical, m_RenderPass, nullptr );
        m_RenderPass = nullptr;
    }

    RenderPassVK RenderPassVK::create(DeviceVK& device, VkFormat format, u32 numSamples, bool depth)
    {
        assert(device.logical && !depth && "Not implemented yet.");
        VkRenderPass renderPass;
        if ( !HelperVK::createRenderPass( device.logical, format, numSamples, renderPass ) )
        {
            return {};
        }
        RenderPassVK rp = {};
        rp.m_Valid  = true;
        rp.m_Device = &device;
        rp.m_RenderPass = renderPass;
        rp.m_Format  = format;
        rp.m_Samples = numSamples;

        return std::move( rp );
    }

    void RenderPassVK::begin(VkCommandBuffer cb, VkFramebuffer fb,
                             const VkOffset2D& offset, const VkExtent2D& extent,
                             const Vector<VkClearValue>& clearValues)
    {
        assert(cb && fb);
        HelperVK::startRenderPass( cb, m_RenderPass, fb, { offset, extent }, clearValues.data() );
    }

    void RenderPassVK::end( VkCommandBuffer cb )
    {
        assert(cb);
        HelperVK::stopRenderPass( cb );
    }
}
#endif
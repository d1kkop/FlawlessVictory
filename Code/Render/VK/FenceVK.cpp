#include "FenceVK.h"
#include "DeviceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    FenceVK::~FenceVK()
    {
        if ( m_Fence )
        {
            vkDestroyFence( m_Device->logical(), m_Fence, NULL );
        }
    }

    M<FenceVK> FenceVK::create( const M<DeviceVK>& device, bool startSignaled )
    {
        VkFence fence;
        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.flags = startSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        if ( VK_SUCCESS != vkCreateFence( device->logical(), &createInfo, NULL, &fence ) )
        {
            LOGC( "VK Failed to create requeste fances." );
            return {};
        }
        auto fenceVk = std::make_shared<FenceVK>();
        fenceVk->m_Fence  = fence;
        fenceVk->m_Device = device;
        return fenceVk;
    }

    void FenceVK::reset()
    {
        VK_CALL_VOID( vkResetFences( m_Device->logical(), 1, &m_Fence ) );
    }

    void FenceVK::wait()
    {
        VK_CALL_VOID( vkWaitForFences( m_Device->logical(), 1, &m_Fence, VK_TRUE, (u64)-1 ) );
    }
}
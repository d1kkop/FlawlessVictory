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
        VkFenceCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.flags = startSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        VK_CALL( vkCreateFence( device->logical(), &createInfo, NULL, &fence ) );
        auto fenceVk = std::make_shared<FenceVK>();
        fenceVk->m_Fence = fence;
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

    void FenceVK::waitForMultiple( const List<M<FenceVK>>& fences )
    {
        static thread_local List<VkFence> fencesList;
        fencesList.clear();
        if ( fences.size() == 0 )
            return;
        fencesList.resize( fences.size() );
        for ( u32 i=0; i < fences.size(); i++ )
            fencesList[i] = fences[i]->vk();
        waitForMultiple( fences[0]->device()->logical(), fencesList );
    }

    void FenceVK::waitForMultiple( VkDevice device, const List<VkFence>& fences )
    {
        VK_CALL_VOID( vkWaitForFences( device, (u32)fences.size(), fences.data(), VK_TRUE, (u64)-1 ) );
    }

}
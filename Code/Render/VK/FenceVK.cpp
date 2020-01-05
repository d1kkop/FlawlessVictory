#include "FenceVK.h"
#include "DeviceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    FenceVK::~FenceVK()
    {
        for ( auto f : m_Fences )
        {
            vkDestroyFence( m_Device->logical(), f, NULL );
        }
    }

    M<FenceVK> FenceVK::create( const M<DeviceVK>& device, bool startSignaled, u32 num )
    {
        List<VkFence> fences;
        for ( u32 i=0; i <num; i++ )
        {
            VkFence fence;
            VkFenceCreateInfo createInfo {};
            createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            createInfo.flags = startSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
            if ( VK_SUCCESS != vkCreateFence( device->logical(), &createInfo, NULL, &fence ) )
            {
                for ( auto f : fences ) vkDestroyFence( device->logical(), f, NULL );
                LOGC( "VK Failed to create requeste fances." );
                return {};
            }
            fences.emplace_back( fence );
        }
        auto fenceVk = std::make_shared<FenceVK>();
        fenceVk->m_Fences = std::move(fences);
        fenceVk->m_Device = device;
        return fenceVk;
    }

    void FenceVK::reset( u32 idx )
    {
        VK_CALL_VOID( vkResetFences( m_Device->logical(), 1, &m_Fences[idx] ) );
    }

    void FenceVK::resetAll()
    {
        VK_CALL_VOID( vkResetFences( m_Device->logical(), (u32)m_Fences.size(), m_Fences.data() ) );
    }

    void FenceVK::wait( u32 idx )
    {
        VK_CALL_VOID( vkWaitForFences( m_Device->logical(), 1, &m_Fences[idx], VK_TRUE, (u64)-1 ) );
    }

    void FenceVK::waitAll()
    {
        waitForMultiple( m_Device->logical(), m_Fences );
    }

    void FenceVK::waitForMultiple( VkDevice device, const List<VkFence>& fences )
    {
        VK_CALL_VOID( vkWaitForFences( device, (u32)fences.size(), fences.data(), VK_TRUE, (u64)-1 ) );
    }

}
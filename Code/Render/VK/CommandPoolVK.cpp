#include "CommandPoolVK.h"
#include "DeviceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    CommandPoolVK::~CommandPoolVK()
    {
        if ( m_CommandPool )
        {
            vkDestroyCommandPool( m_Device->logical(), m_CommandPool, NULL );
        }
    }

    M<CommandPoolVK> CommandPoolVK::create( const M<DeviceVK>& device, u32 forQueueFamily )
    {
        VkCommandPool commandPool;
        VkCommandPoolCreateInfo poolInfo ={};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = forQueueFamily;
        if ( vkCreateCommandPool( device->logical(), &poolInfo, nullptr, &commandPool ) != VK_SUCCESS )
        {
            LOGC( "VK Failed to create command pool." );
            return {};
        }
        auto commPoolVK = std::make_shared<CommandPoolVK>();
        commPoolVK->m_Device = device;
        commPoolVK->m_CommandPool = commandPool;
        return commPoolVK;
    }

    void CommandPoolVK::trimCommandPool()
    {
        vkTrimCommandPool( m_Device->logical(), m_CommandPool, 0 );
    }

    void CommandPoolVK::resetCommandPool()
    {
        VK_CALL_VOID( vkResetCommandPool( m_Device->logical(), m_CommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT ) );
    }

}
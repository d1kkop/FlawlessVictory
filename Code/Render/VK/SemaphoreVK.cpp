#include "SemaphoreVK.h"
#include "DeviceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    SemaphoreVK::~SemaphoreVK()
    {
        if ( m_Semaphore )
        {
            vkDestroySemaphore( m_Device->logical(), m_Semaphore, NULL );
        }
    }

    M<SemaphoreVK> SemaphoreVK::create( const M<DeviceVK>& device )
    {
        VkSemaphore semaphore;
        VkSemaphoreCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.flags = 0;
        VK_CALL( vkCreateSemaphore( device->logical(), &createInfo, NULL, &semaphore ) );
        auto semaphoreVk = std::make_shared<SemaphoreVK>();
        semaphoreVk->m_Semaphore = semaphore;
        semaphoreVk->m_Device = device;
        return semaphoreVk;
    }

}
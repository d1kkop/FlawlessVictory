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
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.flags = 0;
        if ( VK_SUCCESS != vkCreateSemaphore( device->logical(), &createInfo, NULL, &semaphore ) )
        {
            LOGC( "VK Failed to create requested semaphores." );
            return {};
        }
        auto semaphoreVk = std::make_shared<SemaphoreVK>();
        semaphoreVk->m_Semaphore = semaphore;
        semaphoreVk->m_Device = device;
        return semaphoreVk;
    }

}
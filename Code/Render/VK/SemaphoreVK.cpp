#include "SemaphoreVK.h"
#include "DeviceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    SemaphoreVK::~SemaphoreVK()
    {
        for (u32 i = 0; i < m_Semaphores.size() ; i++)
        {
        	vkDestroySemaphore( m_Device->logical(), m_Semaphores[i], NULL );
        }
    }

    M<SemaphoreVK> SemaphoreVK::create( const M<DeviceVK>& device, u32 num )
    {
        List<VkSemaphore> semaphores;
        for ( u32 i=0; i <num; i++ )
        {
            VkSemaphore semaphore;
            VkSemaphoreCreateInfo createInfo {};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            createInfo.flags = 0;
            if ( VK_SUCCESS != vkCreateSemaphore( device->logical(), &createInfo, NULL, &semaphore ) )
            {
                for ( auto& s : semaphores)
                {
                    vkDestroySemaphore( device->logical(), s, NULL );
                }
                LOGC( "VK Failed to create requested semaphores." );
                return {};
            }
            semaphores.emplace_back( semaphore );
        }
        auto semaphoreVk = std::make_shared<SemaphoreVK>();
        semaphoreVk->m_Semaphores = std::move( semaphores );
        semaphoreVk->m_Device = device;
        return semaphoreVk;
    }

}
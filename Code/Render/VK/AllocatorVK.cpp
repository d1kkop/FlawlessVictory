#include "AllocatorVK.h"
#include "DeviceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    AllocatorVK::~AllocatorVK()
    {
        if ( m_Allocator )
        {
            vmaDestroyAllocator( m_Allocator );
        }
    }

    M<AllocatorVK> AllocatorVK::create( const M<DeviceVK>& device )
    {
        VmaAllocator allocator;
        VmaAllocatorCreateInfo allocatorInfo ={};
        allocatorInfo.physicalDevice = device->physical();
        allocatorInfo.device = device->logical();
        if ( vmaCreateAllocator( &allocatorInfo, &allocator ) != VK_SUCCESS )
        {
            LOGC( "VK Failed to create vma allocator." );
            return {};
        }
        M<AllocatorVK> allocatorVK = std::make_shared<AllocatorVK>();
        allocatorVK->m_Allocator = allocator;
        allocatorVK->m_Device = device;
        return allocatorVK;
    }
}
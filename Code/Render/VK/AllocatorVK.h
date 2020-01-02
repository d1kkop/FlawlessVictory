#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class DeviceVK;

    class AllocatorVK
    {
    public:
        AllocatorVK() = default;
        ~AllocatorVK();

    public:
        static M<AllocatorVK> create( const M<DeviceVK>& device );

        VmaAllocator vma() const { return m_Allocator; }
        const M<DeviceVK>& device() const { return m_Device; }

    private:
        VmaAllocator m_Allocator {};
        M<DeviceVK> m_Device;
    };
}
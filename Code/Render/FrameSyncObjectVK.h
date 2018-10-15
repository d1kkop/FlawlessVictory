#pragma once
#include "../Core/Common.h"
#if FV_VULKAN
#include "PCH.h"

namespace fv
{
    struct FrameSyncObjectVK
    {
    public:
        FrameSyncObjectVK() = default;
        void release(); // Do not put in destructor.
        bool create();

        struct DeviceVK* device;
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore imageFinishedSemaphore;
        VkFence frameFence;
    };
}
#endif
#include "FrameSyncObjectVK.h"
#if FV_VULKAN
#include "DeviceVK.h"
#include "../Core/LogManager.h"

namespace fv
{
    void FrameSyncObjectVK::release()
    {
        if (!device ||!device->logical) return;
        vkDestroySemaphore( device->logical, imageAvailableSemaphore, nullptr );
        vkDestroySemaphore( device->logical, imageFinishedSemaphore, nullptr );
        vkDestroyFence( device->logical, frameFence, nullptr );
    }

    bool FrameSyncObjectVK::create()
    {
        assert(device && device->logical && frameFence==nullptr && imageAvailableSemaphore==nullptr && imageFinishedSemaphore==nullptr);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(device->logical, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device->logical, &semaphoreInfo, nullptr, &imageFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(device->logical, &fenceInfo, nullptr, &frameFence) != VK_SUCCESS )
        {
            LOGC("VK Cannot create frame sync objects.");
            return false;
        }
        
        return true;
    }

}
#endif
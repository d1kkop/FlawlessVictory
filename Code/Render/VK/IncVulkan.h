#pragma once
#include "../Core/IncWindows.h"
#define VULKAN_HPP_DISABLE_ENHANCED_MODE
#include <vulkan/vulkan.h>
#include "../3rdParty/VulkanAllocator/vk_mem_alloc.h"

#if FV_DEBUG
#define VK_CALL( exp ) \
    { \
        auto res = exp; \
        if ( res != VK_SUCCESS ) { LOGW( "VK A vulkan call failed with error %d.", res ); return {}; } \
    }
#else
#define VK_CALL( exp )
#endif
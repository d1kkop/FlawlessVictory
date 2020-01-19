#include "DeviceVK.h"
#include "InstanceVK.h"
#include "SurfaceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"
#include "../../Core/IncGLFW.h"

namespace fv
{
    DeviceVK::~DeviceVK()
    {
        if ( m_Logical )
        {
            vkDestroyDevice( m_Logical, NULL );
        }
    }

    M<DeviceVK> DeviceVK::create( const M<InstanceVK>& instance, 
                                  const Vector<const char*>& requiredExtensions,
                                  const Vector<const char*>& requiredLayers,
                                  bool allowIntegratedGPU, 
                                  bool wantGeomtryShader,
                                  bool wantTesselationShader,
                                  u32  numGraphicQueues,
                                  bool wantComputeQueue,
                                  bool wantTransferQueue,
                                  bool wantSparseQueue,
                                  const M<SurfaceVK>& surface )
    {
        // --- Enumerate devices ---

        uint32_t numPhysicalDevice = 0;
        List<VkPhysicalDevice> devices;
        VK_CALL( vkEnumeratePhysicalDevices( instance->vk(), &numPhysicalDevice, NULL ) );
        devices.resize( numPhysicalDevice );
        VK_CALL( vkEnumeratePhysicalDevices( instance->vk(), &numPhysicalDevice, devices.data() ) );

        VkPhysicalDevice chosenPhysical{};
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memProperties;

        // If integrated card is allow, first try to find discrete GPU, otherwise do a second run to find integrated card.
        i32 numTries = 1;
        if ( allowIntegratedGPU ) numTries = 2;
        for ( i32 i = 0; i < numTries; i++ )
        {
            for ( auto physical : devices )
            {
                vkGetPhysicalDeviceFeatures( physical, &features );
                vkGetPhysicalDeviceProperties( physical, &properties );
                vkGetPhysicalDeviceMemoryProperties( physical, &memProperties );
                if ( (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
                     (/* Only allow to find integrated GPU in second run, otheriwse we might find integrated while there is also a discrete card. */
                     i==1 && allowIntegratedGPU && properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)) &&
                     (!wantGeomtryShader || features.geometryShader) &&
                     (!wantTesselationShader || features.tessellationShader) )
                {
                    chosenPhysical = physical;
                    break;
                }
            }
        }

        if ( !chosenPhysical )
        {
            LOGC( "VK Failed to find appropriate physical device." );
            return {};
        }

        // -- Enumerate queues ---

        u32 graphicsFam = -1;
        u32 computeFam  = -1;
        u32 transferFam = -1;
        u32 sparseFam   = -1;
        u32 presentFam  = -1;

        uint32_t queueFamilyCount;
        Vector<VkQueueFamilyProperties> queueFamilies;
        vkGetPhysicalDeviceQueueFamilyProperties( chosenPhysical, &queueFamilyCount, NULL );
        queueFamilies.resize( queueFamilyCount );
        vkGetPhysicalDeviceQueueFamilyProperties( chosenPhysical, &queueFamilyCount, queueFamilies.data() );
        for ( u32 i=0; i<queueFamilyCount; ++i )
        {
            VkQueueFamilyProperties& queueFam = queueFamilies[i];
            if ( queueFam.queueCount > 0 )
            {
                if ( graphicsFam==-1 && (queueFam.queueFlags & VK_QUEUE_GRAPHICS_BIT) ) graphicsFam = i;
                if ( computeFam==-1 && (queueFam.queueFlags & VK_QUEUE_COMPUTE_BIT) )  computeFam  = i;
                if ( transferFam==-1 && (queueFam.queueFlags & VK_QUEUE_TRANSFER_BIT) ) transferFam = i;
                if ( sparseFam==-1 && (queueFam.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ) sparseFam = i;
            }

            // Present support
            if ( surface->vk() && presentFam==-1 )
            {
                VkBool32 presentSupported = false;
                VK_CALL( vkGetPhysicalDeviceSurfaceSupportKHR( chosenPhysical, i, surface->vk(), &presentSupported ) );
                if ( presentSupported ) presentFam = i;
            }
            /*#if FV_GLFW
                if ( glfwGetPhysicalDevicePresentationSupport( instance->vk(), chosenPhysical, i ) )
                {
                    presentFam = i;
                }
            #endif*/
        }

        // Some error checking
        if ( graphicsFam == -1 && numGraphicQueues != 0 )
        {
            LOGC( "VK Cannot find graphic queue family." );
            return {};
        }
        if ( computeFam == -1 && wantComputeQueue )
        {
            LOGC( "VK Cannot find compute queue family." );
            return {};
        }
        if ( transferFam == -1 && wantTransferQueue )
        {
            LOGC( "VK Cannot find transfer queue family." );
            return {};
        }
        if ( sparseFam == -1 && wantSparseQueue )
        {
            LOGC( "VK Cannot find spare queue family." );
            return {};
        }
        if ( presentFam == -1 && surface )
        {
            LOGC( "VK Cannot find present queue family." );
            return {};
        }

        // Obtain unique queue families
        Set<u32> uniqueQueueFamilies;
        if ( graphicsFam != -1 ) uniqueQueueFamilies.insert( graphicsFam );
        if ( computeFam != -1 )  uniqueQueueFamilies.insert( computeFam );
        if ( transferFam != -1 ) uniqueQueueFamilies.insert( transferFam );
        if ( sparseFam != -1 )   uniqueQueueFamilies.insert( sparseFam );
        if ( presentFam != -1 )  uniqueQueueFamilies.insert( presentFam );

        Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float priorities[128] ={};
        assert( uniqueQueueFamilies.size() <= 128 );
        for ( auto& uIdx : uniqueQueueFamilies )
        {
            uint32_t queueCount = (graphicsFam != -1 && uIdx == graphicsFam) ? numGraphicQueues : 1;
            VkDeviceQueueCreateInfo dqci{};
            dqci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            dqci.queueCount = queueCount;
            dqci.queueFamilyIndex = uIdx;
            dqci.pQueuePriorities = priorities;
            queueCreateInfos.emplace_back( dqci );
        }

        VkPhysicalDeviceFeatures deviceFeatures ={};
        VkDeviceCreateInfo createInfo ={};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = (u32)queueCreateInfos.size();
        createInfo.pEnabledFeatures  = &deviceFeatures;
        createInfo.enabledExtensionCount = (u32)requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = (u32)requiredLayers.size();
        createInfo.ppEnabledLayerNames = requiredLayers.data();

        VkDevice logical{};
        VK_CALL( vkCreateDevice( chosenPhysical, &createInfo, NULL, &logical ) );

        M<DeviceVK> device = std::make_shared<DeviceVK>();
        device->m_Physical = chosenPhysical;
        device->m_Logical  = logical;
        device->m_Features = features;
        device->m_Properties = properties;
        device->m_MemProperties = memProperties;

        if ( graphicsFam != -1 )
        {
            for ( u32 i=0; i<numGraphicQueues; ++i )
            {
                VkQueue graphicQueue;
                vkGetDeviceQueue( logical, graphicsFam, i, &graphicQueue );
                device->m_GraphicsQueue.emplace_back( QueueVK::create( graphicQueue, device ) );
            }
        }
        if ( computeFam != -1 && wantComputeQueue )
        {
            VkQueue computeQueue;
            vkGetDeviceQueue( logical, computeFam, 0, &computeQueue );
            device->m_ComputeQueue = QueueVK::create( computeQueue, device );
        }
        if ( transferFam != -1 && wantTransferQueue )
        {
            VkQueue transferQueue;
            vkGetDeviceQueue( logical, transferFam, 0, &transferQueue );
            device->m_TransferQueue = QueueVK::create( transferQueue, device );
        }
        if ( sparseFam != -1 && wantSparseQueue )
        {
            VkQueue sparseQueue;
            vkGetDeviceQueue( logical, sparseFam, 0, &sparseQueue );
            device->m_SparseQueue = QueueVK::create( sparseQueue, device );
        }
        if ( presentFam != -1 && surface )
        {
            VkQueue presentQueue;
            vkGetDeviceQueue( logical, presentFam, 0, &presentQueue );
            device->m_PresentQueue = QueueVK::create( presentQueue, device );
        }

        device->m_GraphicsQueueFam = graphicsFam;
        device->m_ComputeQueueFam  = computeFam;
        device->m_TransferQueueFam = transferFam;
        device->m_SparseQueueFam   = sparseFam;
        device->m_PresentQueueFam  = presentFam;
        
        return device;
    }

    List<u32> DeviceVK::composeQueueFamIndices( bool graphic, bool compute, bool transfer, bool present, bool sparse )
    {
        Set<u32> indices;
        if ( graphic ) indices.insert( m_GraphicsQueueFam );
        if ( compute ) indices.insert( m_ComputeQueueFam );
        if ( transfer ) indices.insert(m_TransferQueueFam );
        if ( present ) indices.insert(m_PresentQueueFam );
        if ( sparse ) indices.insert(m_SparseQueueFam);
        List<u32> lIndices;
        for (auto& i : indices) lIndices.emplace_back(i);
        return std::move(lIndices);
    }

}
#include "RenderManagerVK.h"
#if FV_VULKAN
#include "HelperVK.h"
#include "FrameSyncObjectVK.h"
#include "PipelineVK.h"
#include "RenderImageVK.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
#include "../Core/Directories.h"
#include "../Core/OSLayer.h"

namespace fv
{
    RenderManagerVK::RenderManagerVK()
    {
    }

    RenderManagerVK::~RenderManagerVK()
    {
        closeGraphics();
    }

    bool RenderManagerVK::initGraphics()
    {
        // Setup layers and extensions for instance and devices
    #if FV_DEBUG
        m_RequiredInstanceExtensions = { "VK_EXT_debug_report", "VK_EXT_debug_utils" };
        m_RequiredInstanceLayers = { "VK_LAYER_LUNARG_standard_validation" };
        m_RequiredPhysicalLayers = { "VK_LAYER_LUNARG_standard_validation" };
    #endif
        m_RequiredPhysicalExtensions = { };

        readRenderConfig();
        auto& rc = m_RenderConfig;

        if ( rc.createWindow )
        {
            m_Window = OSCreateWindow(rc.windowName.c_str(), 100, 100, rc.windowWidth, rc.windowHeight, rc.fullScreen, true, false);
            if ( !m_Window )
            {
                LOGC("Failed to create main window %s.", rc.windowName.c_str());
                return false;
            }
            m_RequiredPhysicalExtensions.emplace_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
            HelperVK::queryRequiredWindowsExtensions( m_Window, m_RequiredInstanceExtensions );
        }

        if (!HelperVK::checkRequiredExtensions(m_RequiredInstanceExtensions))
        {
            LOGC("VK Required instance extensions not available.");
            return false;
        }

        if (!HelperVK::checkRequiredLayers(m_RequiredInstanceLayers))
        {
            LOGC("VK Required instance layers not available.");
            return false;
        }

        if ( !HelperVK::createInstance("First App", m_RequiredInstanceExtensions, m_RequiredInstanceLayers, m_Instance) )
        {
            LOGC("VK Failed to create instance.");
            return false;
        }

    #if FV_DEBUG
        if ( !HelperVK::createDebugCallback( m_Instance, false, false, debugCallback, m_DebugCallback ) )
        {
            LOGC("VK Failed to create debug callback.");
            return false;
        }
    #endif

        // In case of main swap chain
        VkSurfaceKHR surface = nullptr;
        if ( m_Window )
        {
            if ( !HelperVK::createSurface(m_Instance, m_Window, surface) )
            {
                LOGC("VK Failed to create main window surface.");
                return false;
            }
        }

        // Note, surface can be null in case there is no mainWindow.
        if ( !createDevices(surface) )
            return false;

        // See if want swap chain
        if ( m_Window )
        {
            // Find device that can present 
            for ( auto& dv : m_Devices )
            {
                if ( dv->createSwapChain( rc, surface ) )
                {
                    if ( !dv->swapChain->createImages( rc.numLayers ) )
                    {
                        LOGC("VK Swap chain creation failed.");
                        return false;
                    }
                    break;
                }
            }
        }

        for ( auto& dv : m_Devices )
        {
            dv->setFormatAndExtent( rc );
            if (!dv->createAllocators() ) return false;
            if (!dv->createStagingBuffers() ) return false;
            if (!dv->createStandard( rc )) return false;
            if (!dv->createCommandPools()) return false;
            if (!dv->createRenderImages( rc )) return false;
            if (!dv->createFrameSyncObjects( rc )) return false;
        }

        // TODO Temp command buffers
        for ( auto& dv : m_Devices )
        {
            dv->recordDrawCommandBuffer( [dv](VkCommandBuffer cb, const RenderImageVK& ri)
            {
                VkClearValue cv = { .4f, .3f, .9f, 0.f };
                HelperVK::startRenderPass(cb, dv->clearPass, ri.frameBuffer, { 0, 0, dv->extent }, &cv);
                vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, dv->clearPipeline.pipeline);
                //vkCmdBindVertexBuffers( cb, 0, 1, 
                vkCmdDraw(cb, 3, 1, 0, 0);
                HelperVK::stopRenderPass(cb);
            });
        }

        LOG("VK Initialized succesful.");
        return true;
    }

    void RenderManagerVK::closeGraphics()
    {
        // Remove debug message handler
        auto destroyDebugUtilMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
        if ( destroyDebugUtilMessenger && m_DebugCallback )
        {
            destroyDebugUtilMessenger( m_Instance, m_DebugCallback, nullptr );
        }
        for ( auto& dv : m_Devices ) 
        {
            dv->release();
            delete dv;
        }
        if ( m_Instance )
        {
            vkDestroyInstance(m_Instance, nullptr);
        }
    }

    void RenderManagerVK::drawFrame()
    {
        for ( auto& dv : m_Devices )
        {
            auto& so = dv->frameSyncObjects[m_FrameImageIdx];
            vkWaitForFences(dv->logical, 1, &so.frameFence, VK_TRUE, (u64)-1);
            vkResetFences(dv->logical, 1, &so.frameFence);

            uint32_t imageIndex; // Iterates from 0 to numImages-1 in swap chain.
            if ( dv->swapChain )
            {
                vkAcquireNextImageKHR(dv->logical, dv->swapChain->swapChain, (u64)-1, so.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
            }
            else
            {
                imageIndex = m_CurrentDrawImage;
            }

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = { so.imageAvailableSemaphore };
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            // Execute command buffers
            auto& ri = dv->renderImages[imageIndex];
            submitInfo.commandBufferCount = (u32)ri.commandBuffers.size();
            submitInfo.pCommandBuffers = ri.commandBuffers.data();

            VkSemaphore signalSemaphores[] = { so.imageFinishedSemaphore };
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            vkQueueSubmit(dv->graphicsQueue, 1, &submitInfo, so.frameFence);

            if ( dv->swapChain )
            {
                VkPresentInfoKHR presentInfo = {};
                presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                presentInfo.waitSemaphoreCount = 1;
                presentInfo.pWaitSemaphores = signalSemaphores;
                presentInfo.swapchainCount = 1;
                presentInfo.pSwapchains = &dv->swapChain->swapChain;
                presentInfo.pImageIndices = &imageIndex;
                vkQueuePresentKHR(dv->presentQueue, &presentInfo);
            }
        }

        // Device indepentent variables
        m_FrameImageIdx = (m_FrameImageIdx + 1) % m_RenderConfig.numFramesBehind;
        m_CurrentDrawImage = (m_CurrentDrawImage + 1) % m_RenderConfig.numImages;
    }

    void RenderManagerVK::waitOnDeviceIdle()
    {
        for ( auto& dv : m_Devices )
        {
            vkDeviceWaitIdle( dv->logical );
        }
    }

    bool RenderManagerVK::createDevices(VkSurfaceKHR surface)
    {
        uint32_t deviceCount = 0;
        Vector<VkPhysicalDevice> physicalDevices;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        physicalDevices.resize( deviceCount );
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data());

        DeviceVK* device = new DeviceVK();
        u32 numCreatedDevices = 0;
        for ( auto& physical : physicalDevices )
        {
            auto& dv = *device;
            dv.instance = m_Instance;
            dv.physical = physical;

            if ( !HelperVK::checkRequiredExtensions(m_RequiredPhysicalExtensions, physical) ||
                 !HelperVK::checkRequiredLayers(m_RequiredPhysicalLayers, physical) )
            {
                continue; // not suitable
            }

            vkGetPhysicalDeviceFeatures(dv.physical, &dv.features);
            vkGetPhysicalDeviceProperties(dv.physical, &dv.properties);
            vkGetPhysicalDeviceMemoryProperties(dv.physical, &dv.memProperties);
            dv.storeDeviceQueueFamilies( surface );

            if ( !((dv.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
                   dv.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) &&
                   dv.features.geometryShader) )
            {
                continue; // Not suitable
            }

            float priorities = 1.f;
            Set<u32> uniqueQueueIndices = { *dv.queueIndices.graphics, *dv.queueIndices.compute, *dv.queueIndices.transfer, *dv.queueIndices.sparse };
            Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            for ( auto& uIdx : uniqueQueueIndices )
            {
                VkDeviceQueueCreateInfo dqci {};
                dqci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                dqci.queueCount = 1;
                dqci.queueFamilyIndex = uIdx;
                dqci.pQueuePriorities = &priorities;
                queueCreateInfos.emplace_back( dqci );
            }

            if ( !HelperVK::createDevice( m_Instance, dv.physical, queueCreateInfos, m_RequiredPhysicalExtensions, m_RequiredPhysicalLayers, dv.logical ) )
            {
                break;
            }

            if ( dv.queueIndices.graphics ) vkGetDeviceQueue(dv.logical, *dv.queueIndices.graphics, 0, &dv.graphicsQueue);
            if ( dv.queueIndices.compute )  vkGetDeviceQueue(dv.logical, *dv.queueIndices.compute, 0, &dv.computeQueue);
            if ( dv.queueIndices.transfer ) vkGetDeviceQueue(dv.logical, *dv.queueIndices.transfer, 0, &dv.transferQueue);
            if ( dv.queueIndices.sparse )   vkGetDeviceQueue(dv.logical, *dv.queueIndices.sparse, 0, &dv.sparseQueue);
            if ( dv.queueIndices.present )  vkGetDeviceQueue(dv.logical, *dv.queueIndices.present, 0, &dv.presentQueue);

            device->idx = numCreatedDevices;
            m_Devices.emplace_back( device );
            device = new DeviceVK();

            if ( ++numCreatedDevices >= m_RenderConfig.maxDevices )
            {
                LOG("VK Possible other suitable devices are discarded as num of max device (%d) was reached.", m_RenderConfig.maxDevices);
                break;
            }
        }

        delete device;
        if ( m_Devices.empty() )
        {
            LOGC("VK Did not find any suitable devices.");
            return false;
        }
        return true;
    }

    u32 RenderManagerVK::autoDeviceIdx()
    {
        u32 retVal = m_AutoDeviceIdx++;
        return retVal % numDevices();
    }

    RTexture2D RenderManagerVK::createTexture2D(u32 deviceIdx, u32 width, u32 height, const char* data, u32 size,
                                                u32 mipLevels, u32 layers, u32 samples, ImageFormat format)
    {
        return m_Devices[deviceIdx]->createTexture2D( width, height, data, size, mipLevels, layers, samples, format, 
                                                     VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY );
    }

    RShader RenderManagerVK::createShader(u32 deviceIdx, const char* data, u32 size)
    {
        return m_Devices[deviceIdx]->createShader( data, size );
    }

    RSubmesh RenderManagerVK::createSubmesh(u32 deviceIdx, const Submesh& submesh, const SubmeshInput& si)
    {
        return m_Devices[deviceIdx]->createInterleavedSubmesh( submesh, si );
    }

    void RenderManagerVK::deleteTexture2D(RTexture2D tex2d)
    {
        if ( tex2d.device == -1 ) return;
        m_Devices[tex2d.device]->deleteTexture2D( tex2d );
    }

    void RenderManagerVK::deleteShader(RShader shader)
    {
        if ( shader.device == -1 ) return;
        m_Devices[shader.device]->deleteShader(shader);
    }

    void RenderManagerVK::deleteSubmesh(RSubmesh submesh)
    {
        if ( submesh.device == -1 ) return;
        m_Devices[submesh.device]->deleteSubmesh(submesh);
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL RenderManagerVK::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                  void* pUserData)
    {
        switch( messageSeverity )
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG( "VK Validation layer %s.", pCallbackData->pMessage );
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            break;
            LOGW( "VK Validation layer %s.", pCallbackData->pMessage );
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOGC( "VK Validation layer %s.", pCallbackData->pMessage );
            break;
        }
        return VK_FALSE;
    }

}
#endif
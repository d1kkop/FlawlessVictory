#include "RenderManagerVK.h"
#if FV_VULKAN
#include "RenderComponent.h"
#include "HelperVK.h"
#include "PipelineVK.h"
#include "RenderImageVK.h"
#include "SubmeshVK.h"
#include "../Core/ComponentManager.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
#include "../Core/Directories.h"
#include "../Core/OSLayer.h"
#include "../Core/JobManager.h"
#include "../Core/GameObject.h"

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
        FV_CHECK_BG();

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
        if ( m_Window )
        {
            if ( !HelperVK::createSurface(m_Instance, m_Window, m_Surface) )
            {
                LOGC("VK Failed to create main window surface.");
                return false;
            }
        }

        // Note, surface can be null in case there is no mainWindow.
        if ( !createDevices(m_Surface, 1 /* jobManager()->numThreads() */ ) )
            return false;

        // See if want swap chain
        if ( m_Window )
        {
            // Find device that can present 
            bool bCreatedSwapChain = false;
            for ( auto& dv : m_Devices )
            {
                if ( dv->createSwapChain( rc, m_Surface ) )
                {
                    bCreatedSwapChain = true;
                    break;
                }
            }
            if ( !bCreatedSwapChain )
            {
                LOGC("VK Swap chain creation failed.");
                return false;
            }
        }

        for ( auto& dv : m_Devices )
        {
            dv->setFormatAndExtent( rc );
            if (!dv->createAllocators() ) return false;
            if (!dv->createStagingBuffers() ) return false;
            if (!dv->createCommandPools()) return false;
            if (!dv->createRenderPasses( rc )) return false;
            if (!dv->createRenderImages( rc )) return false;
            if (!dv->createFrameObjects( rc )) return false;
            if (!dv->createStandard( rc )) return false;
            if (!dv->createPipelines( rc )) return false;
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
        if ( m_Surface ) vkDestroySurfaceKHR( m_Instance, m_Surface, nullptr );
        if ( m_Instance ) vkDestroyInstance(m_Instance, nullptr);
    }

    void RenderManagerVK::drawFrame(const Mat4& view)
    {
        for ( auto& dv : m_Devices )
        {
            for ( auto& rl : dv->renderListsMt ) // render list per thread
            {
                for ( auto& dm : rl ) // draw method
                {
                    for ( auto& pipel : dm ) // pipeline 
                    {
                        pipel.second.clear(); // submeshes list
                    }
                }
            }
        }

        // Fill render lists in multithreaded fashion
        ParallelFlatten<RenderComponent, Vector<ComponentArray>>( componentManager()->drawComponents(), [&](RenderComponent& rc, u32 tIdx)
        {
            rc.cullMT(tIdx);
            if ( !rc.culled() )
            {
                rc.drawMT(tIdx);
            }
        });

        m_RenderJobs.clear();

        for ( auto& dv : m_Devices )
        {
            if ( dv->recreateSwapChain )
            {
                if ( !dv->reCreateSwapChain( m_RenderConfig, m_Surface ) )
                    continue;
                dv->recreateSwapChain = false;
            }

            dv->waitForFences( m_FrameIndex );
            dv->resetFences( m_FrameIndex );

            VkSemaphore waitSemaphore = {};
            u32 renderImageIndex = m_CurrentDrawImage;
            if ( dv->swapChain )
            {
                if ( !dv->swapChain->acquireNextImage( m_FrameIndex, renderImageIndex, waitSemaphore ) )
                {
                    dv->recreateSwapChain = true;
                    continue;
                }
            }

            // Clear standard render image (single threaded)
            VkClearValue cv = { 0, 0, 0, 1 };
            Vector<VkClearValue> cvs = { cv };
            dv->submitGraphicsCommands( m_FrameIndex, 0, waitSemaphore, [&](VkCommandBuffer cb)
            {
                auto& ri = dv->renderImages[renderImageIndex];
                dv->clearColorDepthPass.begin(cb, ri.frameBuffer(), { 0, 0 }, dv->extent, cvs);
                dv->clearColorDepthPass.end(cb);
                return false;
            });

            // For next pass, wait on clear pass to be done
            waitSemaphore = dv->frameFinishSemaphores[m_FrameIndex*(u32)dv->graphicsQueues.size()];

            for ( u32 i=0; i<(u32)dv->graphicsQueues.size(); ++i )
            {
                M<Job> renderJob = jobManager()->addJobOn( i, [&, i]()
                {
                    dv->prepareDrawMethods( m_FrameIndex, i );
                    dv->renderDrawMethod( DrawMethod::FillStandard, m_FrameIndex, i , waitSemaphore );
                });
                m_RenderJobs.emplace_back( renderJob );
            }
        }

        for ( auto& j : m_RenderJobs ) j->wait();

        for ( auto& dv : m_Devices )
        {
            if ( dv->swapChain )
            {
                const VkSemaphore* finishSemaphores = &dv->frameFinishSemaphores[m_FrameIndex*dv->graphicsQueues.size()];
                u32 numSemaphores = (u32)dv->graphicsQueues.size();
                if ( !dv->swapChain->present(finishSemaphores, numSemaphores) )
                {
                    dv->recreateSwapChain = true;
                    continue;
                }
            }
        }

        // Device indepentent variables
        m_FrameIndex = (m_FrameIndex + 1) % m_RenderConfig.numFramesBehind;
        m_CurrentDrawImage = (m_CurrentDrawImage + 1) % m_RenderConfig.numImages;
    }

    void RenderManagerVK::waitOnDeviceIdle()
    {
        for ( auto& dv : m_Devices )
        {
            vkDeviceWaitIdle( dv->logical );
        }
    }

    bool RenderManagerVK::createDevices(VkSurfaceKHR surface, u32 numGraphicsQueues)
    {
        assert( numGraphicsQueues > 0 );

        uint32_t deviceCount = 0;
        Vector<VkPhysicalDevice> physicalDevices;
        FV_VKCALL( vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr) );
        physicalDevices.resize( deviceCount );
        FV_VKCALL( vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data()) );

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

            // 128 must cover number of queues
            assert( numGraphicsQueues <= 128 );
            float priorities [128] = { 1.f };
            Set<u32> uniqueQueueIndices = { *dv.queueIndices.graphics, *dv.queueIndices.compute, *dv.queueIndices.transfer, *dv.queueIndices.sparse };
            Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            for ( auto& uIdx : uniqueQueueIndices )
            {
                VkDeviceQueueCreateInfo dqci {};
                dqci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                dqci.queueCount = (uIdx == dv.queueIndices.graphics.value()) ? numGraphicsQueues : 1;
                dqci.queueFamilyIndex = uIdx;
                dqci.pQueuePriorities = priorities;
                queueCreateInfos.emplace_back( dqci );
            }

            if ( !HelperVK::createDevice( m_Instance, dv.physical, queueCreateInfos, m_RequiredPhysicalExtensions, m_RequiredPhysicalLayers, dv.logical ) )
            {
                break;
            }

            if ( dv.queueIndices.graphics )
            {
                for ( u32 i=0; i< numGraphicsQueues; ++i )
                {
                    VkQueue graphicsQueue;
                    vkGetDeviceQueue(dv.logical, *dv.queueIndices.graphics, i, &graphicsQueue);
                    dv.graphicsQueues.emplace_back( graphicsQueue );
                }
            }
            if ( dv.queueIndices.compute )  vkGetDeviceQueue(dv.logical, *dv.queueIndices.compute, 0, &dv.computeQueue);
            if ( dv.queueIndices.transfer ) vkGetDeviceQueue(dv.logical, *dv.queueIndices.transfer, 0, &dv.transferQueue);
            if ( dv.queueIndices.sparse )   vkGetDeviceQueue(dv.logical, *dv.queueIndices.sparse, 0, &dv.sparseQueue);
            if ( dv.queueIndices.present )  vkGetDeviceQueue(dv.logical, *dv.queueIndices.present, 0, &dv.presentQueue);

            device->idx = numCreatedDevices;
            device->activeGraphicsCmdBuffer.resize( numGraphicsQueues );
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
                                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY );
    }

    RShader RenderManagerVK::createShader(u32 deviceIdx, const char* data, u32 size)
    {
        return m_Devices[deviceIdx]->createShader( data, size );
    }

    RSubmesh RenderManagerVK::createSubmesh(u32 deviceIdx, const Submesh& submesh, const SubmeshInput& si)
    {
        return m_Devices[deviceIdx]->createSubmesh( submesh, si );
    }

    void RenderManagerVK::deleteTexture2D(RTexture2D tex2d)
    {
        if ( tex2d.device == -1 ) return;
        m_Devices[tex2d.device]->deleteTexture2D( tex2d, true );
    }

    void RenderManagerVK::deleteShader(RShader shader)
    {
        if ( shader.device == -1 ) return;
        m_Devices[shader.device]->deleteShader( shader, true );
    }

    void RenderManagerVK::deleteSubmesh(RSubmesh submesh)
    {
        if (!submesh) return;
        SubmeshVK* s = (SubmeshVK*)submesh;
        s->device()->deleteSubmesh( submesh, true );
    }

    void RenderManagerVK::addToRenderList(u32 tIdx, RSubmesh submesh, const MaterialData& mdata, DrawMethod drawMethod)
    {
        if (!submesh) return;
        SubmeshVK* s = (SubmeshVK*)submesh;
        // TODO fix pipeline selection
        PipelineVK* pipeline = s->device()->getOrCreatePipeline( m_PipelineFormatOpaque );
        s->device()->renderListsMt[ tIdx ][ (u32) drawMethod ][ pipeline ].emplace_back( s );
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
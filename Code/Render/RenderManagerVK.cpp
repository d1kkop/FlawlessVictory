#include "RenderManagerVK.h"
#if FV_VULKAN
#include "HelperVK.h"
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
            m_MainWindow = OSCreateWindow(rc.windowName.c_str(), 100, 100, rc.windowWidth, rc.windowHeight, rc.fullScreen, true, false);
            if ( !m_MainWindow )
            {
                LOGC("Failed to create main window %s.", rc.windowName.c_str());
                return false;
            }
            m_RequiredPhysicalExtensions.emplace_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
            HelperVK::queryRequiredWindowsExtensions( m_MainWindow, m_RequiredInstanceExtensions );
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
        if ( m_MainWindow )
        {
            if ( !HelperVK::createSurface(m_Instance, m_MainWindow, m_MainSwapChain.surface) )
            {
                LOGC("VK Failed to create main window surface.");
                return false;
            }
        }

        // Note, surface can be null in case there is no mainWindow.
        if ( !createDevices(m_MainSwapChain.surface) )
            return false;

        // See if want swap chain
        bool bSwapChainCreated = false;
        if ( m_MainWindow )
        {
            // Find device that can present 
            for ( auto& dv : m_Devices )
            {
                if ( createSwapChain( dv, m_MainSwapChain, rc ) )
                {
                    bSwapChainCreated = true;
                    break;
                }
            }
            if ( bSwapChainCreated )
            {
                auto& dv = *m_MainSwapChain.device;
                assert( dv.images.size() == 0 && dv.imgViews.size() == 0 );
                if ( !createImagesFromSwapChain(m_MainSwapChain, rc.numLayers, dv.images, dv.imgViews) )
                    return false;

                // Pick format from swap chain
                dv.format = m_MainSwapChain.surfaceFormat.format;
                dv.extent = m_MainSwapChain.extent;
            }
            else
            {
                LOGC("VK Failed to create main swap chain.");
                return false;
            }
        }

        for ( auto& dv : m_Devices )
        {
            if ( dv.images.empty() ) // No swap chain for device
            {
                if ( !createImagesForDevice( dv, rc ) ) // If not derived from a swap chain.
                    return false;
            }

            // Standard default shaders necessary to set up a pipeline
            if ( !createStandardShaders(dv) )
            {
                return false;
            }

            if ( !HelperVK::createRenderPass(dv.logical, dv.format, rc.numSamples, dv.clearPass) )
            {
                LOGC("Cannot create main render pass. Render setup failed.");
                return false;
            }

            if ( !createFrameBuffersForDevice( dv, dv.clearPass ) )
            {
                return false;
            }

            if ( !createFrameSyncObjects( dv, rc.numFramesBehind ) )
            {
                return false;
            }
        }

        // Temp command buffers
        for ( auto& dv : m_Devices )
        {
            // TEMP create drawable triangle
            u32 oldSize = (u32) dv.commandBuffers.size();
            if ( !HelperVK::allocCommandBuffers(dv.logical, dv.commandPool, 3, dv.commandBuffers) )
            {
                LOGC("Cannot create temporary triangle command buffer.");
                return false;
            }

            u32 i=0;
            for ( auto& cb : dv.commandBuffers )
            {
                if ( !HelperVK::startRecordCommandBuffer( dv.logical, cb ) )
                {
                    LOGC("VK Failed to start record command buffer.");
                    return false;
                }

                VkClearValue cv = { .4f, .3f, .9f, 0.f };
                HelperVK::startRenderPass( cb, dv.clearPass, dv.frameBuffers[i++], { 0, 0, dv.extent }, &cv );
        //        vkCmdBindPipeline( cb, VK_PIPELINE_BIND_POINT_GRAPHICS, dv.opaquePipeline );
                //vkCmdBindVertexBuffers( cb, 0, 1, 
                vkCmdDraw( cb, 3, 1, 0, 0 );
                HelperVK::stopRenderPass( cb );

                if ( !HelperVK::stopRecordCommandBuffer(cb) )
                {
                    LOGC("VK Failed to stop recording command buffer.");
                    return false;
                }
            }
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
            if ( dv.logical )
            {
                if ( dv.standardFrag ) vkDestroyShaderModule( dv.logical, dv.standardFrag, nullptr );
                if ( dv.standardVert ) vkDestroyShaderModule( dv.logical, dv.standardVert, nullptr );
                if ( dv.clearPass ) vkDestroyRenderPass( dv.logical, dv.clearPass, nullptr );
                for ( auto imgView : dv.imgViews )
                {
                    if ( imgView ) vkDestroyImageView( dv.logical, imgView, nullptr );
                }
                if ( !dv.swapChain ) // Otherwise destroyed when swap chain is destroyed.
                {
                    for ( auto img : dv.images )
                    {
                        if ( img ) vkDestroyImage(dv.logical, img, nullptr);
                    }
                }
                for ( auto& kvp : dv.pipelines )
                {
                    auto pipVk = kvp.second;
                    if ( pipVk.pipeline) vkDestroyPipeline( dv.logical, pipVk.pipeline, nullptr );
                    if ( pipVk.layout ) vkDestroyPipelineLayout( dv.logical, pipVk.layout, nullptr );
                }
                for ( auto frameBuffer : dv.frameBuffers )
                {
                    if ( frameBuffer ) vkDestroyFramebuffer( dv.logical, frameBuffer, nullptr );
                }
                if ( dv.commandPool )
                {
                    if ( !dv.swapChain )
                    {
                        for ( auto cb : dv.commandBuffers )
                        {
                            if ( cb ) vkFreeCommandBuffers(dv.logical, dv.commandPool, (u32)dv.commandBuffers.size(), dv.commandBuffers.data());
                        }
                    }
                    vkDestroyCommandPool(dv.logical, dv.commandPool, nullptr);
                }
                for ( auto s : dv.imageAvailableSemaphores ) if ( s ) vkDestroySemaphore( dv.logical, s, nullptr );
                for ( auto s : dv.imageFinishedSemaphores ) if ( s ) vkDestroySemaphore( dv.logical, s, nullptr );
                for ( auto f : dv.frameFences ) if ( f ) vkDestroyFence( dv.logical, f, nullptr );
                if ( m_MainSwapChain.device && m_MainSwapChain.device->logical && m_MainSwapChain.swapChain )
                {
                    vkDestroySwapchainKHR(m_MainSwapChain.device->logical, m_MainSwapChain.swapChain, nullptr);
                }
                if ( m_Instance && m_MainSwapChain.surface )
                {
                    vkDestroySurfaceKHR(m_Instance, m_MainSwapChain.surface, nullptr);
                }
                vkDestroyDevice( dv.logical, nullptr );
            }
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
            vkWaitForFences(dv.logical, 1, &dv.frameFences[m_FrameImageIdx], VK_TRUE, (u64)-1);
            vkResetFences(dv.logical, 1, &dv.frameFences[m_FrameImageIdx]);

            uint32_t imageIndex; // Iterates from 0 to numImages-1 in swap chain.
            if ( dv.swapChain )
            {
                vkAcquireNextImageKHR(dv.logical, m_MainSwapChain.swapChain, (u64)-1, dv.imageAvailableSemaphores[m_FrameImageIdx], VK_NULL_HANDLE, &imageIndex);
            }
            else
            {
                imageIndex = m_CurrentDrawImage;
            }

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = { dv.imageAvailableSemaphores[m_FrameImageIdx] };
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            // Execute command buffers
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &dv.commandBuffers[imageIndex];

            VkSemaphore signalSemaphores[] = { dv.imageFinishedSemaphores[m_FrameImageIdx] };
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            vkQueueSubmit(dv.graphicsQueue, 1, &submitInfo, dv.frameFences[m_FrameImageIdx]);

            if ( dv.swapChain )
            {
                VkPresentInfoKHR presentInfo = {};
                presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

                presentInfo.waitSemaphoreCount = 1;
                presentInfo.pWaitSemaphores = signalSemaphores;

                VkSwapchainKHR swapChains[] = { m_MainSwapChain.swapChain };
                presentInfo.swapchainCount = 1;
                presentInfo.pSwapchains = swapChains;
                presentInfo.pImageIndices = &imageIndex;

                vkQueuePresentKHR(dv.presentQueue, &presentInfo);
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
            vkDeviceWaitIdle( dv.logical );
        }
    }

    bool RenderManagerVK::getOrCreatePipeline(u32 deviceIdx, const SubmeshInput& sinput, const MaterialData& matData, VkRenderPass renderPass, PipelineVK& pipelineOut)
    {
        struct tempStruct 
        {
            SubmeshInput sinput;
            MaterialData matData;
            VkRenderPass renderPass;
        };
        tempStruct ts;
        ts.sinput = sinput;
        ts.matData = matData;
        ts.renderPass = renderPass;
        u32 pipelineHash = Hash32( (const char*)&ts, sizeof(ts) );

     //   VkShaderModule vertShader = 
       
        auto& dv = m_Devices[deviceIdx];

        scoped_lock lk( m_PipelinesMutex );
        auto pIt = dv.pipelines.find(pipelineHash);
        if ( pIt == dv.pipelines.end() )
        {
            // Get input attribs
            u32 vertexSize;
            Vector<VkVertexInputAttributeDescription> inputAttribs;
            HelperVK::createVertexAttribs( sinput, inputAttribs, vertexSize );

            // Create pipeline
            VkPipeline pipeline;
            VkPipelineLayout layout;
            if ( !HelperVK::createPipeline(dv.logical, nullptr, nullptr, nullptr, renderPass, m_MainViewport, inputAttribs, vertexSize, pipeline, layout) )
            {
                LOGW("VK Failed to create pipeline.");
                return false;
            }

            pipelineOut.pipeline;
            pipelineOut.layout = layout;
            dv.pipelines[pipelineHash] = pipelineOut;
        }
        else
        {
            pipelineOut = pIt->second;
        }

        return true;
    }

    bool RenderManagerVK::createDevices(VkSurfaceKHR surface)
    {
        uint32_t deviceCount = 0;
        Vector<VkPhysicalDevice> physicalDevices;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        physicalDevices.resize( deviceCount );
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data());

        u32 numCreatedDevices = 0;
        for ( auto& physical : physicalDevices )
        {
            DeviceVK dv {};
            dv.physical = physical;

            if ( !HelperVK::checkRequiredExtensions(m_RequiredPhysicalExtensions, physical) ||
                 !HelperVK::checkRequiredLayers(m_RequiredPhysicalLayers, physical) )
            {
                continue; // not suitable
            }

            vkGetPhysicalDeviceFeatures(dv.physical, &dv.features);
            vkGetPhysicalDeviceProperties(dv.physical, &dv.properties);
            vkGetPhysicalDeviceMemoryProperties(dv.physical, &dv.memProperties);
            HelperVK::storeDeviceQueueFamilies( dv, surface );

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
                LOGC( "VK Failed to create logical devices." );
                return false;
            }

            if ( !HelperVK::createCommandPool( dv.logical, dv.queueIndices.graphics.value(), dv.commandPool ))
            {
                LOGC( "VK Failed to create command pool.");
                return false;
            }

            if ( dv.queueIndices.graphics ) vkGetDeviceQueue(dv.logical, *dv.queueIndices.graphics, 0, &dv.graphicsQueue);
            if ( dv.queueIndices.compute )  vkGetDeviceQueue(dv.logical, *dv.queueIndices.compute, 0, &dv.computeQueue);
            if ( dv.queueIndices.transfer ) vkGetDeviceQueue(dv.logical, *dv.queueIndices.transfer, 0, &dv.transferQueue);
            if ( dv.queueIndices.sparse )   vkGetDeviceQueue(dv.logical, *dv.queueIndices.sparse, 0, &dv.sparseQueue);
            if ( dv.queueIndices.present )  vkGetDeviceQueue(dv.logical, *dv.queueIndices.present, 0, &dv.presentQueue);

            m_Devices.push_back( dv );

            if ( ++numCreatedDevices >= m_RenderConfig.maxDevices )
            {
                LOG("VK Possible other suitable devices are discarded as num of max device (%d) was reached.", m_RenderConfig.maxDevices);
                break;
            }
        }

        if ( m_Devices.empty() )
        {
            LOGC("VK Did not find any suitable devices.");
            return false;
        }
        return true;
    }

    bool RenderManagerVK::createSwapChain(DeviceVK& device, SwapChainVK& swapChain, const RenderConfig& rc)
    {
        assert( device.logical && device.physical && swapChain.surface );
        if (!HelperVK::createSwapChain(device.logical, device.physical, swapChain.surface, 
                                       rc, 
                                       device.queueIndices.graphics,
                                       device.queueIndices.present,
                                       swapChain.surfaceFormat, swapChain.presentMode, swapChain.extent,
                                       swapChain.swapChain) )
        {
            LOGC("VK Failed to create swap chain.");
            return false;
        }
        swapChain.device = &device;
        device.swapChain = &swapChain;
        return true;
    }

    bool RenderManagerVK::createImagesFromSwapChain(const SwapChainVK& swapChain, u32 numLayers, Vector<VkImage>& images, Vector<VkImageView>& imgViews)
    {
        // Retrieve swap chain images
        assert( swapChain.device && swapChain.device->logical && swapChain.device && swapChain.device->logical );
        uint32_t swapChainImgCount;
        vkGetSwapchainImagesKHR(swapChain.device->logical, swapChain.swapChain, &swapChainImgCount, nullptr);
        images.resize(swapChainImgCount);
        vkGetSwapchainImagesKHR(swapChain.device->logical, swapChain.swapChain, &swapChainImgCount, images.data());

        // Create image views on images in swap chain
        for ( auto& img : images )
        {
            VkImageView imgView;
            if ( !HelperVK::createImageView(swapChain.device->logical, img, swapChain.surfaceFormat.format, numLayers, imgView) )
            {
                LOGW("VK Cannot create all images for swap chain. Render setup failed.");
                return false;
            }
            imgViews.emplace_back(imgView);
        }

        assert(imgViews.size() == images.size());
        return true;
    }

    bool RenderManagerVK::createImagesForDevice(DeviceVK& dv, const RenderConfig& rc)
    {
        assert(dv.images.empty() && dv.imgViews.empty() && rc.numImages > 0 && rc.numSamples >= 1);
        for ( u32 i=0; i< rc.numImages; ++i )
        {
            VkImageCreateInfo ici {};
            ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            ici.flags = 0;
            ici.imageType = VK_IMAGE_TYPE_2D;
            ici.format = VK_FORMAT_B8G8R8A8_UNORM;
            ici.extent = { rc.resX, rc.resY, 1 };
            ici.mipLevels = 1;
            ici.arrayLayers = rc.numLayers;
            ici.samples = (VkSampleCountFlagBits) rc.numSamples;
            ici.tiling = VK_IMAGE_TILING_OPTIMAL;
            ici.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            ici.queueFamilyIndexCount = 1; 
            ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            uint32_t queueFamIndices [] = { dv.queueIndices.graphics.value() };
            ici.pQueueFamilyIndices = queueFamIndices;
            VkImage image;
            if ( vkCreateImage(dv.logical, &ici, nullptr, &image) != VK_SUCCESS )
            {
                LOGC("VK Cannot create image for device.");
                return false;
            }
            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(dv.logical, image, &memRequirements);
            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = HelperVK::findMemoryType(memRequirements.memoryTypeBits, dv.memProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VkDeviceMemory imageMemory;
            if ( vkAllocateMemory(dv.logical, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS )
            {
                LOGC("VK Failed to allocate memory for image.");
                return false;
            }
            if ( vkBindImageMemory(dv.logical, image, imageMemory, 0) != VK_SUCCESS )
            {
                vkFreeMemory(dv.logical, imageMemory, nullptr);
                LOGC("VK Failed to bind image memory.");
                return false;
            }
            // Format and extent from image create params
            dv.format = ici.format;
            dv.extent = { ici.extent.width, ici.extent.height };
            dv.images.emplace_back(image);
            VkImageView imgView;
            if ( !HelperVK::createImageView(dv.logical, image, ici.format, rc.numLayers, imgView) )
            {
                LOGC("VK Cannot create image view for device.");
                return false;
            }
            dv.imgViews.emplace_back(imgView);
        }
        return true;
    }

    bool RenderManagerVK::createFrameBuffersForDevice(DeviceVK& device, VkRenderPass renderPass)
    {
        assert( renderPass && device.logical );
        // For each image, there is a framebuffer and ascociated img views.
        u32 i=0;
        for ( auto& img : device.images )
        {
            VkFramebuffer frameBuffer;
            Vector<VkImageView> attachments = { device.imgViews[i++] };
            if ( !HelperVK::createFramebuffer(device.logical, device.extent, renderPass, attachments, frameBuffer) )
            {
                LOGC("VK Cannot create all frame buffers for swap chain. Render setup failed.");
                return false;
            }
            device.frameBuffers.emplace_back(frameBuffer);
        }
        return true;
    }

    bool RenderManagerVK::createStandardShaders(DeviceVK& dv)
    {
        if ( !HelperVK::createShaderFromBinary( dv.logical, Directories::standard() / "standard.frag.spv", dv.standardFrag) )
        {
            LOGC("VK Cannot create standard frag shader. Render setup failed.");
            return false;
        }
        if ( !HelperVK::createShaderFromBinary(dv.logical, Directories::standard() / "standard.vert.spv", dv.standardVert) )
        {
            LOGC("VK Cannot create standard vert shader. Render setup failed.");
            return false;
        }
        return true;
    }

    bool RenderManagerVK::createFrameSyncObjects(DeviceVK& device, u32 numMaxFramesBehind)
    {
        assert( device.logical && device.frameFences.size()==0 && device.imageAvailableSemaphores.size()==0 && device.imageFinishedSemaphores.size()==0 && numMaxFramesBehind>= 1 );

        device.imageAvailableSemaphores.resize(numMaxFramesBehind);
        device.imageFinishedSemaphores.resize(numMaxFramesBehind);
        device.frameFences.resize(numMaxFramesBehind);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for ( u32 i = 0; i < numMaxFramesBehind; i++ )
        {
            if (vkCreateSemaphore(device.logical, &semaphoreInfo, nullptr, &device.imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device.logical, &semaphoreInfo, nullptr, &device.imageFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device.logical, &fenceInfo, nullptr, &device.frameFences[i]) != VK_SUCCESS )
            {
                LOGC("VK Cannot create frame sync objects.");
                return false;
            }
        }
        return true;
    }

    u64 RenderManagerVK::createTexture2D(u32 width, u32 height, const char* data, u32 size, ImageFormat format)
    {
        assert(false);
        return -1;
    }

    u64 RenderManagerVK::createShader(const char* data, u32 size)
    {
        assert(false);
        return -1;
    }

    u64 RenderManagerVK::createSubmesh(const Submesh& submesh, const MaterialData& matData)
    {
        assert(false);
        return -1;
    }

    void RenderManagerVK::deleteTexture2D(u64 tex2d)
    {
        assert(false);
    }

    void RenderManagerVK::deleteShader(u64 shader)
    {
        assert(false);
    }

    void RenderManagerVK::deleteSubmesh(u64 submesh)
    {
        assert(false);
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
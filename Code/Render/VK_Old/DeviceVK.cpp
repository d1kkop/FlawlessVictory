#include "DeviceVK.h"
#if FV_VULKAN
#include "RenderManager.h" // For config
#include "HelperVK.h"
#include "SwapChainVK.h"
#include "BufferVK.h"
#include "ImageVK.h"
#include "SubmeshVK.h"
#include "../Core/Directories.h"
#include "../Core/LogManager.h"
#include "../Core/Functions.h"
#include "../Core/Algorithm.h"
#include "../Core/Thread.h"
#include "../Core/JobManager.h"

namespace fv
{
    constexpr u32 STAGING_BUFFER_SIZE = 1024*1024*64; // 64 MB

    DeviceVK* DeviceVK::create(VkInstance instance, VkPhysicalDevice physical, u32 idx, VkSurfaceKHR surface, u32 numGraphicQueues,
                               const Vector<const char*>& physicalExtensions,
                               const Vector<const char*>& physicalLayers)
    {
        assert( instance && physical && numGraphicQueues > 0 );

        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memProperties;

        vkGetPhysicalDeviceFeatures(physical, &features);
        vkGetPhysicalDeviceProperties(physical, &properties);
        vkGetPhysicalDeviceMemoryProperties(physical, &memProperties);

        if ( !((properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
                properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) &&
               features.geometryShader) )
        {
            LOGC("VK Found a graphic device, but required features not available. Discarding device.");
            return nullptr;
        }

        QueueFamilyIndicesVK queueIndices;
        HelperVK::getQueueIndices(physical, surface, queueIndices);

        VkQueue computeQueue= {}, transferQueue= {}, sparseQueue= {}, presentQueue= {};
        Set<u32> uniqueQueueIndices;
        if ( queueIndices.compute )     uniqueQueueIndices.insert(queueIndices.compute.value());
        if ( queueIndices.transfer )    uniqueQueueIndices.insert(queueIndices.transfer.value());
        if ( queueIndices.sparse )      uniqueQueueIndices.insert(queueIndices.sparse.value());
        if ( queueIndices.present )     uniqueQueueIndices.insert(queueIndices.present.value());

        Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float priorities[128] = {};
        assert( uniqueQueueIndices.size() <= 128 );
        for ( auto& uIdx : uniqueQueueIndices )
        {
            VkDeviceQueueCreateInfo dqci {};
            dqci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            dqci.queueCount = (queueIndices.graphics.has_value () && uIdx == queueIndices.graphics.value()) ? numGraphicQueues : 1;
            dqci.queueFamilyIndex = uIdx;
            dqci.pQueuePriorities = priorities;
            queueCreateInfos.emplace_back(dqci);
        }

        VkDevice logical;
        if (!HelperVK::createDevice(instance, physical, queueCreateInfos, physicalExtensions, physicalLayers, logical))
        {
            return nullptr;
        }

        DeviceVK* d = new DeviceVK{};
        assert( d->idx==-1 );
        d->idx = idx;
        d->physical = physical;
        d->logical = logical;
        d->properties = properties;
        d->features = features;
        d->memProperties = memProperties;
        d->queueIndices = queueIndices;
        
        if ( queueIndices.graphics )
        {
            for ( u32 i=0; i<numGraphicQueues; ++i )
            {
                VkQueue graphicQueue;
                vkGetDeviceQueue( logical, queueIndices.graphics.value(), i, &graphicQueue );
                d->graphicsQueues.emplace_back( graphicQueue );
            }
        }
        if ( queueIndices.compute ) vkGetDeviceQueue( logical, queueIndices.compute.value(), 0, &d->computeQueue );
        if ( queueIndices.transfer ) vkGetDeviceQueue( logical, queueIndices.transfer.value(), 0, &d->transferQueue );
        if ( queueIndices.sparse ) vkGetDeviceQueue( logical, queueIndices.sparse.value(), 0, &d->sparseQueue );

        d->renderListsMt.resize(numGraphicQueues);
        for ( auto& rl : d->renderListsMt )
        {
            rl.resize( (u32)DrawMethod::Count );
        }

        return d;
    }

    void DeviceVK::release()
    {
        for ( auto& tex2d: textures2d ) deleteTexture2D( tex2d, false );
        for ( auto& shad: shaders) deleteShader( shad, false );
        for ( auto& subm : submeshes) deleteSubmesh( subm, false );
        for ( auto& qs : frameFinishSemaphores ) for (auto& s : qs) vkDestroySemaphore(logical, s, nullptr);
        for ( auto& qs : frameFinishFences ) for (auto& f : qs) vkDestroyFence(logical, f, nullptr);
        for ( auto& cp : graphicsPools ) vkDestroyCommandPool(logical, cp, nullptr);
        stageBuffer.release();
        releaseSwapchain();
        vkDestroyShaderModule( logical, standardFrag, nullptr );
        vkDestroyShaderModule( logical, standardVert, nullptr );
        vkDestroyCommandPool( logical, transferPool, nullptr );
        vmaDestroyAllocator( allocator );
        vkDestroyDevice( logical, nullptr );
    }

    void DeviceVK::releaseSwapchain()
    {
        delete swapChain; swapChain = nullptr;
        for ( auto& ri : renderImages ) ri.release();
        renderImages.clear();
        clearColorDepthPass.release();
        // Resource thread may try to add new pipeline while recreating swap chain. Do lock.
        {
            raii_lock lk(pipelineMutex);
            for ( auto& kvp : pipelines ) kvp.second.release();
            pipelines.clear();
        }
    }

    void DeviceVK::setFormatAndExtent(const RenderConfig& rc)
    {
        if ( swapChain )
        {
            format = swapChain->surfaceFormat().format;
            extent = swapChain->extent();
        }
        else
        {
            format = VK_FORMAT_B8G8R8A8_UNORM;
            extent = { rc.resX, rc.resY };
        }
    }

    bool DeviceVK::createAllocators()
    {
        assert( physical && logical );
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = physical;
        allocatorInfo.device = logical;
        if ( vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS )
        {
            LOGC("VK Failed to create vma allocator.");
            return false;
        }
        return true;
    }

    bool DeviceVK::createCommandPools()
    {
        assert( logical && queueIndices.graphics && graphicsPools.empty() && queueIndices.transfer && transferPool==nullptr && graphicsQueues.size()>0 );
        if ( !HelperVK::createCommandPool(logical, queueIndices.transfer.value(), transferPool) )
        {
            return false;
        }
        for ( u32 i=0; i<(u32)graphicsQueues.size(); ++i )
        {
            VkCommandPool graphicsPool;
            if ( !HelperVK::createCommandPool(logical, queueIndices.graphics.value(), graphicsPool) )
            {
                return false;
            }
            graphicsPools.emplace_back( graphicsPool );
        }
        return true;
    }

    bool DeviceVK::createStagingBuffers()
    {
        VmaMemoryUsage vmaUsage = (VmaMemoryUsage) ( VMA_MEMORY_USAGE_CPU_TO_GPU );
        VkBufferUsageFlagBits vkUsage = (VkBufferUsageFlagBits) ( VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | 
                                                                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT );
        u32 queueIdxs [] = { queueIndices.graphics.value(), queueIndices.transfer.value() };
        stageBuffer = BufferVK::create( *this, STAGING_BUFFER_SIZE, vkUsage, vmaUsage, queueIdxs, 2, nullptr );
        if ( !stageBuffer.valid() )
        {
            LOGC("VK Failed to create staging buffer(s).");
            return false;
        }
        return true;
    }

    bool DeviceVK::createSwapChain(const RenderConfig& rc, VkSurfaceKHR surface)
    {
        assert(logical && physical && surface && !swapChain);
        swapChain = SwapChainVK::create(*this, surface, rc.numFramesBehind, rc.windowWidth, rc.windowHeight, rc.numImages, rc.numLayers, 
                                          queueIndices.graphics, queueIndices.present, nullptr);
        return swapChain != nullptr;
    }

    bool DeviceVK::createRenderPasses(const RenderConfig& rc)
    {
        clearColorDepthPass = RenderPassVK::create(*this, 2, 1, 0, [&](u32 idx, VkAttachmentDescription& atd, VkAttachmentReference& atr)
        {
            if ( idx == 0 ) // Color
            {
                atd.format  = format;
                atd.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
                atd.finalLayout = VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR;
                // attachment ref 
                atr.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            else if ( idx == 1 ) // Depth
            {
                atd.format  = HelperVK::findDepthFormat( physical );
                atd.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
                atd.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                // attachment ref 
                atr.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
        }, [](u32 idx, VkSubpassDescription& sd, const Vector<VkAttachmentReference>& refs)
        {
            sd.colorAttachmentCount = 1;
            sd.pColorAttachments = refs.data();
            sd.pDepthStencilAttachment = &refs.back();
        }, [](u32 idx, VkSubpassDependency& dp) {});

        if ( !clearColorDepthPass.valid() )
        {
            LOGC("Cannot create main render pass. Render setup failed.");
            return false;
        }

        RenderPassVK standardPass = RenderPassVK::create(*this, 2, 1, 0, [&](u32 idx, VkAttachmentDescription& atd, VkAttachmentReference& atr)
        {
            if ( idx == 0 ) // Color
            {
                atd.format  = format;
                atd.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                atd.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                // attachment ref 
                atr.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            else if ( idx == 1 ) // Depth
            {
                atd.format = HelperVK::findDepthFormat( physical );
                atd.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                atd.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                // attachment ref 
                atr.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
        }, [](u32 idx, VkSubpassDescription& sd, const Vector<VkAttachmentReference>& refs)
        {
            sd.colorAttachmentCount = 1;
            sd.pColorAttachments = refs.data();
            sd.pDepthStencilAttachment = &refs.back();
        }, [](u32 idx, VkSubpassDependency& sd) {} );

        if ( !standardPass.valid() )
        {
            LOGC("Cannot create standard pass. Render setup failed.");
            return false;
        }

        drawMethods.resize( (u32)DrawMethod::Count );
        drawMethods[ (u32)DrawMethod::FillStandard ] = standardPass;

        return true;
    }

    bool DeviceVK::reCreateSwapChain(const struct RenderConfig& rc, VkSurfaceKHR surface)
    {
        assert( logical && surface );
        vkDeviceWaitIdle( logical );
        releaseSwapchain();
        if ( createSwapChain( rc, surface ) )
        {
            createRenderPasses( rc );
            createRenderImages( rc );
        }
        return swapChain != nullptr;
    }

    bool DeviceVK::createRenderImages( const struct RenderConfig& rc )
    {
        assert(renderImages.size()==0 && rc.numImages > 0 && rc.numSamples >= 1);
        renderImages.resize(swapChain ? swapChain->images().size() : rc.numImages);
        u32 i=0;
        for ( auto& ri : renderImages )
        {
            bool cbFailed = false;
            bool bSetup = ri.initialize( *this, clearColorDepthPass.renderPass(), [&](u32 idx, VkImage& img, VkFormat& format, VkImageAspectFlags& flags, u32& numLayers)
            {
                if ( idx == 0 )
                {
                    numLayers = rc.numLayers;
                    flags = VK_IMAGE_ASPECT_COLOR_BIT;
                    if ( swapChain )
                    {
                        img = swapChain->images()[i++];
                        format = swapChain->surfaceFormat().format;
                    }
                    else
                    {
                        ImageVK* colorImg = (ImageVK*) createTexture2D( rc.resX, rc.resY, nullptr, 0, 1, rc.numLayers, rc.numSamples, this->format, 
                                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY );
                        if ( !colorImg )
                        {
                            cbFailed = true;
                            LOGC("VK Failed to create render color image.");
                            return false;
                        }
                        img = colorImg->image();
                        format = colorImg->format();
                    }
                    return true; // continue for another image (depth)
                }
                else if (idx == 1)
                {
                    VkFormat depthFormat = HelperVK::findDepthFormat( physical );
                    ImageVK* depthImg = (ImageVK*)createTexture2D(rc.resX, rc.resY, nullptr, 0, 1, rc.numLayers, rc.numSamples, depthFormat,
                                                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                    if ( !depthImg )
                    {
                        cbFailed = true;
                        LOGC("VK Failed to create render depth image.");
                        return false;
                    }
                    img = depthImg->image();
                    format = depthImg->format();
                    flags = flags = VK_IMAGE_ASPECT_DEPTH_BIT;
                    numLayers = rc.numLayers;
                    return false; // stop
                }
                return false;
            });
            if (!bSetup || cbFailed) return false;
        }
        return true;
    }

    bool DeviceVK::createFrameObjects(const struct RenderConfig& rc)
    {
        assert( rc.numFramesBehind >= 1 );

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        frameFinishSemaphores.resize(graphicsQueues.size());
        frameFinishFences.resize(graphicsQueues.size());
        frameGraphicsCmds.resize(graphicsQueues.size());
        for ( u32 i=0; i<(u32)graphicsQueues.size(); ++i )
        {
            frameGraphicsCmds[i].resize(rc.numFramesBehind);
            for ( u32 i=0; i<rc.numFramesBehind; ++i )
            {
                VkSemaphore imageFinished;
                VkFence frameFence;

                if ( vkCreateSemaphore( logical, &semaphoreInfo, nullptr, &imageFinished ) != VK_SUCCESS ||
                     vkCreateFence( logical, &fenceInfo, nullptr, &frameFence ) != VK_SUCCESS )
                {
                    LOGC( "VK Cannot create frame sync objects." );
                    return false;
                }

                frameFinishSemaphores[i].emplace_back( imageFinished );
                frameFinishFences[i].emplace_back( frameFence );
            }
        }

        return true;
    }

    bool DeviceVK::createPipelines(const RenderConfig& rc)
    {
        assert( standardFrag && standardVert );

        MaterialData md = {};
        md.fragShader = { idx, standardFrag };
        md.vertShader = { idx, standardVert };

        SubmeshInput si = {};
        si.positions = true;
        si.normals = true;
        si.uvs = true;

        PipelineFormatVK pipelineFormat = {};
        pipelineFormat.mdata  = md;
        pipelineFormat.sinput = si;
        pipelineFormat.cullmode   = VK_CULL_MODE_NONE;
        pipelineFormat.frontFace  = VK_FRONT_FACE_CLOCKWISE;
        pipelineFormat.polyMode   = VK_POLYGON_MODE_FILL;
        pipelineFormat.lineWidth  = 1.0f;
        pipelineFormat.numSamples = rc.numSamples;
        pipelineFormat.renderPass = clearColorDepthPass.renderPass();

        //if ( !getOrCreatePipeline( pipelineFormat, pipelineOpaqueStandard ))
        //{
        //    LOGC("VK Cannot create opaque standard pipeline.");
        //    return false;
        //}

        //pipelineFormat.sinput.tanBins = true;
        //if ( !getOrCreatePipeline( pipelineFormat, pipelineOpaqueStandardTB ))
        //{
        //    LOGC("VK Cannot create opaque standard tanbin pipeline.");
        //    return false;
        //}

        //pipelineFormat.sinput.bones = true;
        //if ( !getOrCreatePipeline(pipelineFormat, pipelineOpaqueStandardTBBones) )
        //{
        //    LOGC("VK Cannot create opaque standard tanbin with bones pipeline.");
        //    return false;
        //}

        //pipelineFormat.sinput.tanBins = false;
        //if ( !getOrCreatePipeline(pipelineFormat, pipelineOpaqueStandardBones) )
        //{
        //    LOGC("VK Cannot create opaque standard with bones pipeline.");
        //    return false;
        //}

        return true;
    }

    bool DeviceVK::createStandard(const RenderConfig& rc)
    {
        if ( !HelperVK::createShaderFromBinary(logical, Directories::standard() / "standard.frag.spv", standardFrag) )
        {
            LOGC("VK Cannot create standard frag shader. Render setup failed.");
            return false;
        }
        if ( !HelperVK::createShaderFromBinary(logical, Directories::standard() / "standard.vert.spv", standardVert) )
        {
            LOGC("VK Cannot create standard vert shader. Render setup failed.");
            return false;
        }
        return true;
    }

    PipelineVK* DeviceVK::getOrCreatePipeline(u32 pipelineHash, const PipelineFormatVK& format)
    {
        raii_lock lk(pipelineMutex);
        auto pIt = pipelines.find(pipelineHash);
        if ( pIt == pipelines.end() )
        {
            // Get input attribs
            u32 vertexSize;
            Vector<VkVertexInputAttributeDescription> vertexAttribs;
            HelperVK::createVertexAttribs(format.sinput, vertexAttribs, vertexSize);

            VkVertexInputBindingDescription vertexBinding {};
            vertexBinding.binding = 0;
            vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            vertexBinding.stride = vertexSize;
            Vector<VkVertexInputBindingDescription> vertexBindings;
            if ( !vertexAttribs.empty() )
                vertexBindings.emplace_back(vertexBinding);

            // Conduct default viewport (Scissor is deduced from this, it can be changed at anytime).
            VkViewport vp;
            vp.x = 0;
            vp.y = 0;
            vp.width  = (float)extent.width;
            vp.height = (float)extent.height;
            vp.minDepth = 0.f;
            vp.maxDepth = 1.f;

            // Create pipeline
            PipelineVK pipeline = PipelineVK::create(*this, pipelineHash, format, vp, vertexBindings, vertexAttribs);
            if ( !pipeline.valid() )
            {
                return nullptr;
            }

            pipelines[pipelineHash] = std::move(pipeline);
            return &pipelines[pipelineHash];
        }
        else
        {
            return &pIt->second;
        }

        return nullptr;
    }

    PipelineVK* DeviceVK::getOrCreatePipeline(const PipelineFormatVK& format)
    {
        u32 pipelineHash = Hash32((const char*)&format, sizeof(format));
        return getOrCreatePipeline( pipelineHash, format );
    }

    bool DeviceVK::mapStagingBuffer(BufferVK& staging, void** pMapped)
    {
        assert( pMapped );
        stageBufferMtx.lock();
        if ( !stageBuffer.map( pMapped ) || !(*pMapped) )
        {
            stageBufferMtx.unlock();
            LOGC("VK Failed to map staging buffer.");
            return false;
        }
        staging = stageBuffer;
        return true;
    }

    void DeviceVK::unmapStagingBuffer()
    {
        stageBuffer.unmap();
        stageBuffer.flush();
        stageBufferMtx.unlock();
    }

    void DeviceVK::waitForFrameFinishFences(u32 frameIndex)
    {
        for ( auto& fencesPerQueue : frameFinishFences )
        {
            while ( vkWaitForFences( logical, 1, &fencesPerQueue[frameIndex], VK_TRUE, -1 ) == VK_TIMEOUT );
        }
    }

    void DeviceVK::resetFrameFinishFences(u32 frameIndex)
    {
        for ( auto& fencesPerQueue : frameFinishFences )
        {
            FV_VKCALL( vkResetFences( logical, 1, &fencesPerQueue[frameIndex] ) );
        }
    }

    void DeviceVK::prepareDrawMethods(u32 frameIdx, u32 tIdx)
    {
        // Clear previous command buffers for this frameIndex [ 0 to N ], where N is usually 1, 2 or 3.
        u32 offs = frameIdx*(u32)graphicsQueues.size();
        HelperVK::freeCommandBuffers(logical, graphicsPools[tIdx], frameGraphicsCmds[offs + tIdx].data(), (u32)frameGraphicsCmds[offs +tIdx].size());
        frameGraphicsCmds[offs +tIdx].clear();
    }

    void DeviceVK::renderDrawMethod(DrawMethod drawMethod, VkFramebuffer fb, u32 frameIdx, u32 tIdx, VkSemaphore waitSemaphore)
    {
        //assert( fb );
        //HashMap<PipelineVK*, Vector<SubmeshVK*>>& pipeLines = renderListsMt[tIdx][(u32)drawMethod];
        //submitGraphicsCommands(frameIdx, tIdx, waitSemaphore, [&, tIdx](VkCommandBuffer cb)
        //{
        //    drawMethods[ (u32) drawMethod ].begin( cb, fb, { 0, 0 }, extent, {} );
        //    for ( auto& pipel : pipeLines )
        //    {
        //        pipel.first->bind( cb );
        //        for ( auto& s : pipel.second ) // submeshes
        //        {
        //            s->render( cb );
        //        }
        //    }
        //    drawMethods[ (u32)drawMethod ].end( cb );
        //    return true; // is last submit
        //});
    }

    void DeviceVK::submitGraphicsCommand( u32 frameIndex, u32 queueIdx,
                                          VkCommandBuffer commandBuffer,
                                          VkSemaphore waitSemaphore, VkSemaphore signalSemaphore,
                                          bool submit,
                                          const Function<bool( VkCommandBuffer cb )>& callback )
    {
        callback( commandBuffer );

        if ( submit )
        {

            VkSubmitInfo submitInfo ={};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            if ( waitSemaphore )
            {
                VkSemaphore waitSemaphores[] ={ waitSemaphore };
                submitInfo.waitSemaphoreCount = 1;
                submitInfo.pWaitSemaphores = waitSemaphores;
                VkPipelineStageFlags waitStages[] ={ VK_PIPELINE_STAGE_TRANSFER_BIT };
                submitInfo.pWaitDstStageMask = waitStages;
            }

            if ( signalSemaphore )
            {
                VkSemaphore signalSemaphores[] ={ signalSemaphore };
                submitInfo.signalSemaphoreCount = 1;
                submitInfo.pSignalSemaphores = signalSemaphores;
            }

            auto& gq = graphicsQueues[queueIdx];
            FV_VKCALL( vkQueueSubmit( gq, 1, &submitInfo, VK_NULL_HANDLE ) );
        }
    }

    void DeviceVK::submitOnetimeTransferCommand(const Function<void (VkCommandBuffer)>& callback)
    {
        assert(logical && transferQueue && transferPool && callback );

        VkCommandBuffer cb;
        HelperVK::allocCommandBuffer( logical, transferPool, cb );
        HelperVK::beginCommandBuffer( logical, (VkCommandBufferUsageFlags) VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, cb );

        callback( cb );

        HelperVK::endCommandBuffer( cb );
  
        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.pCommandBuffers = &cb;
        submit.commandBufferCount = 1;
      //  VkPipelineStageFlags stageFlags [] = { VK_PIPELINE_STAGE_TRANSFER_BIT };
      //  submit.pWaitDstStageMask = stageFlags; // TODO is this necessary ?
        FV_VKCALL( vkQueueSubmit(transferQueue, 1, &submit, VK_NULL_HANDLE) );
        FV_VKCALL( vkQueueWaitIdle( transferQueue ) );

        HelperVK::freeCommandBuffers( logical, transferPool, &cb, 1 );
    }

    RTexture2D DeviceVK::createTexture2D(u32 width, u32 height, const char* data, u32 size,
                                         u32 mipLevels, u32 layers, u32 samples, ImageFormat format,
                                         VkImageUsageFlagBits usageBits, VmaMemoryUsage memUsage)
    {
        VkFormat vkFormat = HelperVK::convert( format );
        return createTexture2D( width, height, data, size, mipLevels, layers, samples, vkFormat, usageBits, memUsage);
    }

    RTexture2D DeviceVK::createTexture2D(u32 width, u32 height, const char* data, u32 size,
                                         u32 mipLevels, u32 layers, u32 samples, VkFormat format,
                                         VkImageUsageFlagBits usageBits, VmaMemoryUsage memUsage)
    {
        assert(width > 0 && height > 0 && mipLevels > 0 && layers > 0 && samples > 0 && usageBits != 0);
        DeviceResource dr = { idx };
        u32 queueIdxes[] = { queueIndices.graphics.value() };
        ImageVK* image = ImageVK::create(*this, width, height, 1, format, VK_IMAGE_TILING_OPTIMAL /* TODO CHECK */, mipLevels, layers, samples,
                                         queueIdxes, 1, usageBits, memUsage, nullptr);
        if ( !image )
        {
            return {};
        }
        if ( data ) 
        {
            // upload .. TODO 
        }
        raii_lock lk(tex2dMutex);
        textures2d.emplace_back(image);
        return image;
    }

    RShader DeviceVK::createShader(const char* data, u32 size)
    {
        DeviceResource dr = { idx };
        if ( !HelperVK::createShaderModule( logical, data, size, *(VkShaderModule*)&dr.resources ) )
        {
            return {};
        }
        raii_lock lk(shaderMutex);
        shaders.emplace_back( dr );
        return dr;
    }

    RSubmesh DeviceVK::createSubmesh(const Submesh& submesh, const SubmeshInput& si)
    {
        u32 vCount = submesh.getVertexCount();
        if ( vCount == 0 || submesh.indices.size()== 0 )
        {
            LOGW("VK Failed to create submesh as submesh contains no indices or vertices.");
            return {};
        }
        u32 numComponents = si.computeNumComponents();
        u32 bufferSize = numComponents * 4 * vCount;
        if ( bufferSize > STAGING_BUFFER_SIZE )
        {
            LOGW("VK Requested vertex buffer size exceeds staging buffer size (%d bytes). Cannot upload data to GPU.", STAGING_BUFFER_SIZE);
            return {};
        }

        // Make interleaved vertex buffer.
        char* vertices = new char[bufferSize];
        char* ptr = vertices;
        for ( u32 i=0; i<vCount; ++i )
        {
            if ( si.positions )
            {
                memcpy( ptr, (const char*)&submesh.vertices[i].x, sizeof(Vec3) );
                ptr += sizeof(Vec3);
            }
            if ( si.normals ) 
            {
                memcpy( ptr, (const char*)&submesh.normals[i].x, sizeof(Vec3) ); 
                ptr += sizeof(Vec3);
            }
            if ( si.tanBins )
            {
                memcpy( ptr, (const char*)&submesh.tangents[i].x, sizeof(Vec3) );
                ptr += sizeof(Vec3);
                memcpy( ptr, (const char*)&submesh.bitangents[i].x, sizeof(Vec3) );
                ptr += sizeof(Vec3);
            }
            if ( si.uvs )
            {
                memcpy(ptr, (const char*)&submesh.uvs[i].x, sizeof(Vec2));
                ptr += sizeof(Vec2);
            }
            if ( si.lightUvs )
            {
                memcpy(ptr, (const char*)&submesh.lightUVs[i].x, sizeof(Vec2));
                ptr += sizeof(Vec2);
            }
            const Vector<Vec4>* extras [] = { &submesh.extra1, &submesh.extra2, &submesh.extra3, &submesh.extra4 };
            for ( u32 j=0; j<4; ++j )
            {
                if ( si.extras[j] )
                {
                    memcpy(ptr, (const char*)&(*extras[j])[i].x, sizeof(Vec4));
                    ptr += sizeof(Vec4);
                }
            }
            if ( si.bones )
            {
                memcpy(ptr, (const char*)&submesh.weights[i].x, sizeof(Vec4));
                ptr += sizeof(Vec4);
                memcpy(ptr, (const char*)&submesh.boneIndices[i], sizeof(u32));
                ptr += sizeof(float);
            }
        }

        SubmeshVK* submeshVK = SubmeshVK::create(*this, 
                                                 vertices, vCount, numComponents*4,
                                                 submesh.indices.data(), (u32) submesh.indices.size(),
                                                 si);
        delete [] vertices;
        if ( !submeshVK )
        {
            return {};
        }
        
        raii_lock lk(submeshMutex);
        submeshes.emplace_back( submeshVK );

        return submeshVK;
    }

    void DeviceVK::deleteTexture2D(RTexture2D tex2d, bool removeFromList)
    {
        ImageVK* img = (ImageVK*)tex2d;
        delete img;
        raii_lock lk(tex2dMutex);
        if ( removeFromList ) RemoveMemCmp( textures2d, tex2d );
    }

    void DeviceVK::deleteShader(RShader shader, bool removeFromList)
    {
        vkDestroyShaderModule( logical, (VkShaderModule) shader.resources[0], nullptr );
        raii_lock lk(shaderMutex);
        if ( removeFromList ) RemoveMemCmp( shaders, shader );
    }

    void DeviceVK::deleteSubmesh(RSubmesh submesh, bool removeFromList)
    {
        SubmeshVK* s = (SubmeshVK*)submesh;
        delete s;
        raii_lock lk(submeshMutex);
        if ( removeFromList ) Remove( submeshes, submesh );
    }

}
#endif
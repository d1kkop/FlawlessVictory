#include "DeviceVK.h"
#if FV_VULKAN
#include "RenderManager.h" // For config
#include "HelperVK.h"
#include "SwapChainVK.h"
#include "BufferVK.h"
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

    void DeviceVK::release()
    {
        for ( auto& tex2d: textures2d ) deleteTexture2D( tex2d, false );
        for ( auto& shad: shaders) deleteShader( shad, false );
        for ( auto& subm : submeshes) deleteSubmesh( subm, false );
        for ( auto& s : frameFinishSemaphores ) vkDestroySemaphore(logical, s, nullptr);
        for ( auto& f : frameFinishFences ) vkDestroyFence(logical, f, nullptr);
        for ( auto& gp : graphicsPools ) vkDestroyCommandPool(logical, gp, nullptr);
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
            scoped_lock lk(pipelineMutex);
            for ( auto& kvp : pipelines ) kvp.second.release();
            pipelines.clear();
        }
    }

    void DeviceVK::storeDeviceQueueFamilies(VkSurfaceKHR surface)
    {
        assert(physical); // Surface may be null. In that case no present queue is obtained.
        uint32_t queueFamilyCount;
        Vector<VkQueueFamilyProperties> queueFamilies;
        vkGetPhysicalDeviceQueueFamilyProperties(physical, &queueFamilyCount, nullptr);
        queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physical, &queueFamilyCount, queueFamilies.data());
        for ( u32 i=0; i<queueFamilyCount; ++i )
        {
            auto& queueFam = queueFamilies[i];
            if ( queueFam.queueCount > 0 )
            {
                if ( (queueFam.queueFlags & VK_QUEUE_GRAPHICS_BIT) ) queueIndices.graphics = i;
                if ( (queueFam.queueFlags & VK_QUEUE_COMPUTE_BIT) )  queueIndices.compute = i;
                if ( (queueFam.queueFlags & VK_QUEUE_TRANSFER_BIT) ) queueIndices.transfer = i;
                if ( (queueFam.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ) queueIndices.sparse = i;
            }
            VkBool32 presentSupported = false;
            if ( surface )
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, surface, &presentSupported);
                if ( presentSupported ) queueIndices.present = i;
            }
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

        standardPass = RenderPassVK::create(*this, 2, 1, 0, [&](u32 idx, VkAttachmentDescription& atd, VkAttachmentReference& atr)
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

    bool DeviceVK::createRenderImages(const struct RenderConfig& rc)
    {
        assert(renderImages.size()==0 && rc.numImages > 0 && rc.numSamples >= 1);
        renderImages.resize(swapChain ? swapChain->images().size() : rc.numImages);
        u32 i=0;
        for ( auto& ri : renderImages )
        {
            VkImage swapChainImg = swapChain ? swapChain->images()[i++] : nullptr;
            if ( !ri.initialize( *this, rc, swapChainImg, clearColorDepthPass.renderPass() ) )
            {
                return false;
            }
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

        for ( u32 i=0; i<rc.numFramesBehind; ++i )
        {
            for ( u32 j=0; j<(u32)graphicsQueues.size(); ++j )
            {
                VkSemaphore imageFinished;
                VkFence frameFence;

                if ( vkCreateSemaphore(logical, &semaphoreInfo, nullptr, &imageFinished) != VK_SUCCESS ||
                     vkCreateFence(logical, &fenceInfo, nullptr, &frameFence) != VK_SUCCESS )
                {
                    LOGC("VK Cannot create frame sync objects.");
                    return false;
                }

                frameFinishSemaphores.emplace_back(imageFinished);
                frameFinishFences.emplace_back(frameFence);
            }
        }

        frameGraphicsCmds.resize( rc.numFramesBehind * graphicsQueues.size() );
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
        scoped_lock lk(pipelineMutex);
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

    void DeviceVK::waitForFences(u32 frameIndex)
    {
        while ( vkWaitForFences(logical, (u32)graphicsQueues.size(), 
                                frameFinishFences.data()+frameIndex*graphicsQueues.size(),
                                VK_TRUE, -1) == VK_TIMEOUT );
    }

    void DeviceVK::resetFences(u32 frameIdex)
    {
        FV_VKCALL(vkResetFences(logical, (u32)graphicsQueues.size(),
                                frameFinishFences.data()+frameIdex*graphicsQueues.size()));
    }

    void DeviceVK::prepareDrawMethods(u32 frameIdx, u32 tIdx)
    {
        // Clear previous command buffers for this frameIndex [ 0 to N ], where N is usually 1, 2 or 3.
        u32 offs = frameIdx*(u32)graphicsQueues.size();
        HelperVK::freeCommandBuffers(logical, graphicsPools[tIdx], frameGraphicsCmds[offs + tIdx].data(), (u32)frameGraphicsCmds[offs +tIdx].size());
        frameGraphicsCmds[offs +tIdx].clear();
    }

    void DeviceVK::renderDrawMethod(DrawMethod drawMethod, u32 frameIdx, u32 tIdx, VkSemaphore waitSemaphore)
    {
        HashMap<PipelineVK*, Vector<SubmeshVK*>>& pipeLines = renderListsMt[tIdx][(u32)drawMethod];
        submitGraphicsCommands(frameIdx, tIdx, waitSemaphore, [&, tIdx](VkCommandBuffer cb)
        {
            // TODO render pass begin from draw method
            for ( auto& pipel : pipeLines )
            {
                pipel.first->bind( cb );
                for ( auto& s : pipel.second ) // submeshes
                {
                    s->render( cb );
                }
            }
            // TODO render pass end from draw method
            return false;
        });
    }

    void DeviceVK::submitGraphicsCommands(u32 frameIndex, u32 queueIdx, VkSemaphore waitSemaphore,
                                          const Function<bool (VkCommandBuffer cb)>& callback)
    {
        auto gq = graphicsQueues[queueIdx];
        auto gp = graphicsPools[queueIdx];
        u32 arrIdx = frameIndex*(u32)graphicsQueues.size()+queueIdx;
        VkSemaphore doneSemaphore = frameFinishSemaphores[arrIdx];
        VkFence doneFence = frameFinishFences[arrIdx];
        Vector<VkCommandBuffer>& frameCmds = frameGraphicsCmds[arrIdx];
        assert( frameCmds.empty() );

        VkCommandBuffer cb;
        HelperVK::allocCommandBuffer(logical, gp, cb);
        HelperVK::beginCommandBuffer(logical, (VkCommandBufferUsageFlags)VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, cb);
        activeGraphicsCmdBuffer[ queueIdx ] = cb;
        bool lastSubmit = callback(cb);
        HelperVK::endCommandBuffer(cb);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        if ( waitSemaphore )
        {
            VkSemaphore waitSemaphores[] = { waitSemaphore };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TRANSFER_BIT };
            submitInfo.pWaitDstStageMask = waitStages;
        }

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cb;

        if ( !lastSubmit )
        {
            FV_VKCALL(vkQueueSubmit(gq, 1, &submitInfo, VK_NULL_HANDLE));
        }
        else
        {
            VkSemaphore signalSemaphores[]  = { doneSemaphore };
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;
            auto res = vkQueueSubmit(gq, 1, &submitInfo, doneFence);
            assert(res == VK_SUCCESS);
        }

        frameCmds.emplace_back(cb);
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
        assert( width > 0 && height > 0 && data && size >= width*height && mipLevels > 0 && layers > 0 && samples > 0 && usageBits != 0 );
        VkFormat vkFormat = HelperVK::convert( format );
        DeviceResource dr = { idx };
        if (!MemoryHelperVK::createImage(*this, width, height, vkFormat, mipLevels, layers, samples, 
                                         false, queueIndices.graphics.value(), usageBits, memUsage, *(ImageVK*) &dr.resources))
        {
            return {};
        }
        scoped_lock lk(tex2dMutex);
        textures2d.emplace_back( dr );
        return dr;
    }

    RShader DeviceVK::createShader(const char* data, u32 size)
    {
        DeviceResource dr = { idx };
        if ( !HelperVK::createShaderModule( logical, data, size, *(VkShaderModule*)&dr.resources ) )
        {
            return {};
        }
        scoped_lock lk(shaderMutex);
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
        
        scoped_lock lk(submeshMutex);
        submeshes.emplace_back( submeshVK );

        return submeshVK;
    }

    void DeviceVK::deleteTexture2D(RTexture2D tex2d, bool removeFromList)
    {
        MemoryHelperVK::freeImage( *(ImageVK*) &tex2d.resources );
        scoped_lock lk(tex2dMutex);
        if ( removeFromList ) RemoveMemCmp( textures2d, tex2d );
    }

    void DeviceVK::deleteShader(RShader shader, bool removeFromList)
    {
        vkDestroyShaderModule( logical, (VkShaderModule) shader.resources[0], nullptr );
        scoped_lock lk(shaderMutex);
        if ( removeFromList ) RemoveMemCmp( shaders, shader );
    }

    void DeviceVK::deleteSubmesh(RSubmesh submesh, bool removeFromList)
    {
        SubmeshVK* s = (SubmeshVK*)submesh;
        delete s;
        scoped_lock lk(submeshMutex);
        if ( removeFromList ) Remove( submeshes, submesh );
    }

}
#endif
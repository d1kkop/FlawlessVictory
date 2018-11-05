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
        delete m_SwapChain; m_SwapChain = nullptr;
        for ( auto& ri : renderImages ) ri.release();
        for ( auto& tex2d: textures2d ) deleteTexture2D( tex2d, false );
        for ( auto& shad: shaders) deleteShader( shad, false );
        for ( auto& subm : submeshes) deleteSubmesh( subm, false );
        for ( auto& kvp : pipelines ) kvp.second.release();
        // clearPipeline.release(); // No need, is in map of pipelines 
        m_StagingBuffer.release();
        clearColorDepthPass.release();
        vkDestroyShaderModule( logical, standardFrag, nullptr );
        vkDestroyShaderModule( logical, standardVert, nullptr );
        vkDestroyCommandPool( logical, transferPool, nullptr );
        for ( auto& s : frameFinishSemaphores) vkDestroySemaphore( logical, s, nullptr );
        for ( auto& f : frameFinishFences ) vkDestroyFence( logical, f, nullptr );
        for ( auto& gp : graphicsPools ) vkDestroyCommandPool( logical, gp, nullptr );
        vmaDestroyAllocator( allocator );
        vkDestroyDevice( logical, nullptr );
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
        if ( m_SwapChain )
        {
            format = m_SwapChain->surfaceFormat().format;
            extent = m_SwapChain->extent();
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
        m_StagingBuffer = BufferVK::create( *this, STAGING_BUFFER_SIZE, vkUsage, vmaUsage, queueIdxs, 2, nullptr );
        if ( !m_StagingBuffer.valid() )
        {
            LOGC("VK Failed to create staging buffer(s).");
            return false;
        }
        return true;
    }

    bool DeviceVK::createSwapChain(const RenderConfig& rc, VkSurfaceKHR surface)
    {
        assert(logical && physical && surface && !m_SwapChain);
        m_SwapChain = SwapChainVK::create(*this, surface, rc.numFramesBehind, rc.windowWidth, rc.windowHeight, rc.numImages, rc.numLayers, 
                                          queueIndices.graphics, queueIndices.present);
        return m_SwapChain != nullptr;
    }

    bool DeviceVK::createRenderImages(const struct RenderConfig& rc)
    {
        assert(renderImages.size()==0 && rc.numImages > 0 && rc.numSamples >= 1);
        renderImages.resize(swapChain() ? swapChain()->images().size() : rc.numImages);
        u32 i=0;
        for ( auto& ri : renderImages )
        {
            VkImage swapChainImg = swapChain() ? swapChain()->images()[i++] : nullptr;
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

    bool DeviceVK::createStandard(const RenderConfig& rc)
    {
        clearColorDepthPass = RenderPassVK::create(*this, format, rc.numSamples, false);
        if ( !clearColorDepthPass.valid() )
        {
            LOGC("Cannot create main render pass. Render setup failed.");
            return false;
        }
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

    bool DeviceVK::getOrCreatePipeline(u32 pipelineHash, const PipelineFormatVK& format, PipelineVK& pipelineOut)
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
            if ( vertexAttribs.size() )
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
            pipelineOut = PipelineVK::create(*this, pipelineHash, format, vp, vertexBindings, vertexAttribs);
            if ( !pipelineOut.valid() )
            {
                return false;
            }

            pipelines[pipelineHash] = std::move(pipelineOut);
        }
        else
        {
            pipelineOut = pIt->second;
        }

        return true;
    }

    bool DeviceVK::getOrCreatePipeline(const PipelineFormatVK& format, PipelineVK& pipelineOut)
    {
        u32 pipelineHash = Hash32((const char*)&format, sizeof(format));
        return getOrCreatePipeline( pipelineHash, format, pipelineOut );
    }

    bool DeviceVK::mapStagingBuffer(BufferVK& staging, void** pMapped)
    {
        assert( pMapped );
        m_StagingBufferMutex.lock();
        if ( !m_StagingBuffer.map( pMapped ) || !(*pMapped) )
        {
            m_StagingBufferMutex.unlock();
            LOGC("VK Failed to map staging buffer.");
            return false;
        }
        staging = m_StagingBuffer;
        return true;
    }

    void DeviceVK::unmapStagingBuffer()
    {
        m_StagingBuffer.unmap();
        m_StagingBuffer.flush();
        m_StagingBufferMutex.unlock();
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

    void DeviceVK::submitGraphicsCommands(u32 frameIndex, u32 queueIdx, VkSemaphore waitSemaphore, const Function<bool (VkCommandBuffer cb)>& callback)
    {
        bool bContinue = false;
        bool isFirstSubmit = true;
        auto gq = graphicsQueues[queueIdx];
        auto gp = graphicsPools[queueIdx];
        u32 arrIdx = frameIndex*(u32)graphicsQueues.size()+queueIdx;
        VkSemaphore doneSemaphore = frameFinishSemaphores[arrIdx];
        VkFence doneFence = frameFinishFences[arrIdx];
        Vector<VkCommandBuffer>& frameCmds = frameGraphicsCmds[arrIdx];
        assert( frameCmds.empty() );

        do
        {
            VkCommandBuffer cb;
            HelperVK::allocCommandBuffer(logical, gp, cb);
            HelperVK::beginCommandBuffer( logical, (VkCommandBufferUsageFlags) VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, cb );
            bContinue = callback( cb );
            HelperVK::endCommandBuffer(cb);

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            if ( waitSemaphore && isFirstSubmit )
            {
                VkSemaphore waitSemaphores[] = { waitSemaphore };
                submitInfo.waitSemaphoreCount = 1;
                submitInfo.pWaitSemaphores = waitSemaphores;
                VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TRANSFER_BIT };
                submitInfo.pWaitDstStageMask = waitStages;
            }

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cb;

            if ( bContinue )
            {
                FV_VKCALL( vkQueueSubmit(gq, 1, &submitInfo, VK_NULL_HANDLE) );
            }
            else
            {
                VkSemaphore signalSemaphores[]  = { doneSemaphore };
                submitInfo.signalSemaphoreCount = 1;
                submitInfo.pSignalSemaphores = signalSemaphores;
                FV_VKCALL( vkQueueSubmit(gq, 1, &submitInfo, doneFence) );
                FV_VKCALL( vkQueueWaitIdle( gq ) );
            }

            frameCmds.emplace_back( cb );

        } while ( bContinue );
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
        char* vertexBuffer = new char[bufferSize];
        char* ptr = vertexBuffer;
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

        SubmeshVK* submeshVK = SubmeshVK::create( *this, vertexBuffer, bufferSize, submesh.indices.data(), (u32) submesh.indices.size()*sizeof(u32), si, numComponents*4 );
        delete [] vertexBuffer;
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
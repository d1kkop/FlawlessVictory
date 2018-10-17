#include "DeviceVK.h"
#if FV_VULKAN
#include "RenderManager.h" // For config
#include "HelperVK.h"
#include "SwapChainVK.h"
#include "../Core/Directories.h"
#include "../Core/LogManager.h"
#include "../Core/Functions.h"
#include "../Core/Algorithm.h"

namespace fv
{
    constexpr u32 STAGING_BUFFER_SIZE = 1024*1024*64; // 64 MB

    void DeviceVK::release()
    {
        if ( swapChain ) swapChain->release();
        delete swapChain; swapChain = nullptr;
        for ( auto& ri : renderImages ) ri.release();
        for ( auto& fo : frameSyncObjects) fo.release();
        for ( auto& tex2d: textures2d ) deleteTexture2D( tex2d );
        for ( auto& shad: shaders) deleteShader( shad );
        for ( auto& subm : submeshes ) deleteSubmesh( subm );
        for ( auto& kvp : pipelines ) kvp.second.release();
        // clearPipeline.release(); // No need, is in map of pipelines 
        vkDestroyRenderPass( logical, clearPass, nullptr );
        vkDestroyShaderModule( logical, standardFrag, nullptr );
        vkDestroyShaderModule( logical, standardVert, nullptr );
        vkDestroyCommandPool( logical, commandPool, nullptr );
        if ( stagingBuffer ) 
        {
            MemoryHelperVK::freeBuffer( *stagingBuffer );
            delete stagingBuffer;
        }
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
        if ( swapChain )
        {
            format = swapChain->surfaceFormat.format;
            extent = swapChain->extent;
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
        assert( logical && queueIndices.graphics && commandPool==nullptr );
        if ( !HelperVK::createCommandPool(logical, queueIndices.graphics.value(), commandPool) )
        {
            return false;
        }
        return true;
    }

    bool DeviceVK::createStagingBuffers()
    {
        assert( !stagingBuffer );
        stagingBuffer = new BufferVK;
        VkBufferUsageFlagBits vkFlags = (VkBufferUsageFlagBits) ( VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );
        VmaMemoryUsage vmaFlags = (VmaMemoryUsage) ( VMA_MEMORY_USAGE_CPU_TO_GPU | VMA_MEMORY_USAGE_GPU_TO_CPU );
        if ( !MemoryHelperVK::createBuffer(*this, STAGING_BUFFER_SIZE, vkFlags, vmaFlags, *stagingBuffer) )
        {
            delete stagingBuffer;
            stagingBuffer = nullptr;
            LOGC("VK Failed to create staging buffer.");
            return false;
        }
        return true;
    }

    bool DeviceVK::createSwapChain(const RenderConfig& rc, VkSurfaceKHR surface)
    {
        assert(logical && physical && surface && !swapChain);
        swapChain = new SwapChainVK();
        swapChain->surface = surface;
        swapChain->device = this;
        if ( !HelperVK::createSwapChain(logical, physical, surface,
                                        rc,
                                        queueIndices.graphics,
                                        queueIndices.present,
                                        swapChain->surfaceFormat, swapChain->presentMode, swapChain->extent,
                                        swapChain->swapChain) )
        {
            // NOTE: Creation may fail if required settings are not available.
            swapChain->release();
            delete swapChain;
            swapChain = nullptr;
            return false;
        }
        return true;
    }

    bool DeviceVK::createStandard( const RenderConfig& rc )
    {
        if ( !HelperVK::createRenderPass(logical, format, rc.numSamples, clearPass) )
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
        SubmeshInput sinput{};
        MaterialData mdata{};
        mdata.fragShader = { idx, standardFrag };
        mdata.vertShader = { idx, standardVert };
        if ( !getOrCreatePipeline( sinput, mdata, clearPass, clearPipeline ) )
        {
            LOGC("VK Cannot create standard pipeline.");
            return false;
        }
        return true;
    }

    bool DeviceVK::createRenderImages(const struct RenderConfig& rc)
    {
        assert(renderImages.size()==0 && rc.numImages > 0 && rc.numSamples >= 1);
        renderImages.resize(rc.numImages);
        u32 i=0;
        for ( auto& ri : renderImages )
        {
            ri.device = this;
            VkImage swapChainImg = nullptr;
            if ( !ri.device->swapChain )
            {
                if ( !ri.createImage( rc ) ) return false;
            } 
            else
            {
                assert( swapChain->images.size() == rc.numImages );
                swapChainImg = ri.device->swapChain->images[i++];
            }
            if ( !ri.createImageView( rc, swapChainImg ) ) return false;
            if ( !ri.createFrameBuffer( clearPass ) ) return false;
        }
        return true;
    }

    bool DeviceVK::createFrameSyncObjects(const struct RenderConfig& rc)
    {
        assert(frameSyncObjects.size() == 0 && rc.numFramesBehind >= 1 );
        frameSyncObjects.resize( rc.numFramesBehind );
        for ( auto& fso : frameSyncObjects )
        {
            fso.device = this;
            if ( !fso.create() ) return false;
        }
        return true;
    }

    bool DeviceVK::getOrCreatePipeline(const SubmeshInput& sinput, const MaterialData& matData, VkRenderPass renderPass, PipelineVK& pipelineOut)
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
        u32 pipelineHash = Hash32((const char*)&ts, sizeof(ts));

        scoped_lock lk(pipelineMutex);
        auto pIt = pipelines.find(pipelineHash);
        if ( pIt == pipelines.end() )
        {
            // Get input attribs
            u32 vertexSize;
            Vector<VkVertexInputAttributeDescription> vertexAttribs;
            HelperVK::createVertexAttribs(sinput, vertexAttribs, vertexSize);

            VkVertexInputBindingDescription vertexBinding {};
            vertexBinding.binding = 0;
            vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            vertexBinding.stride = vertexSize;
            Vector<VkVertexInputBindingDescription> vertexBindings;
            if ( vertexAttribs.size() )
                vertexBindings.emplace_back( vertexBinding );

            // Conduct default viewport
            VkViewport vp;
            vp.x = 0;
            vp.y = 0;
            vp.width  = (float)extent.width;
            vp.height = (float)extent.height;
            vp.minDepth = 0.f;
            vp.maxDepth = 1.f;

            // Create pipeline
            VkPipeline pipeline;
            VkPipelineLayout layout;
            if ( !HelperVK::createPipeline(logical, 
                                           (VkShaderModule) matData.vertShader.resources[0],
                                           (VkShaderModule) matData.fragShader.resources[0],
                                           (VkShaderModule) matData.geomShader.resources[0],
                                           renderPass, vp, vertexBindings, vertexAttribs, vertexSize, pipeline, layout) )
            {
                // pipelineMutex is already unlocked
                return false;
            }

            pipelineOut.device = this;
            pipelineOut.pipeline = pipeline;
            pipelineOut.layout = layout;

            pipelines[pipelineHash] = pipelineOut;
        }
        else
        {
            pipelineOut = pIt->second;
        }
        return true;
    }

    void DeviceVK::recordDrawCommandBuffer(const Function<void (VkCommandBuffer, const RenderImageVK&)>& recordCb)
    {
        assert( logical && commandPool );
        for ( auto& ri : renderImages )
        {
            VkCommandBuffer cb;
            HelperVK::allocCommandBuffer(logical, commandPool, cb);
            ri.commandBuffers.emplace_back( cb );
            HelperVK::startRecordCommandBuffer(logical, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, cb);
            // Record actual command buffer by using a callback
            recordCb( cb, ri );
            HelperVK::stopRecordCommandBuffer(cb);
        }
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

    RSubmesh DeviceVK::createInterleavedSubmesh(const Submesh& submesh, const SubmeshInput& si)
    {
        u32 vCount = submesh.getVertexCount();
        if ( vCount == 0 || submesh.indices.size()== 0 )
        {
            LOGW("VK Failed to create submesh as submesh contains no indices or vertices.");
            return {};
        }
        u32 numComponents  = si.computeNumComponents();
        u32 bufferSize = numComponents * 4 * vCount;
        if ( bufferSize > STAGING_BUFFER_SIZE )
        {
            LOGW("VK Requested vertex buffer size exceeds staging buffer size (%d bytes). Cannot upload data to GPU.", STAGING_BUFFER_SIZE);
            return {};
        }
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
        BufferVK deviceVertexBuffer;
        VkBufferUsageFlagBits vkFlags = (VkBufferUsageFlagBits) (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT );
        VmaMemoryUsage vmaFlags = (VmaMemoryUsage) VMA_MEMORY_USAGE_GPU_ONLY;
        if ( !MemoryHelperVK::createBuffer( *this, bufferSize, vkFlags, vmaFlags, deviceVertexBuffer ) )
        {
            delete [] vertexBuffer;
            return {};
        }
        MemoryHelperVK::copyToStagingBuffer( *this, vertexBuffer, bufferSize );
        delete [] vertexBuffer;
        MemoryHelperVK::copyBuffer( *this, deviceVertexBuffer, *stagingBuffer );
        RSubmesh rSubmes;
        rSubmes.device = idx;
        rSubmes.resources[0] = deviceVertexBuffer.allocation;
        rSubmes.resources[1] = deviceVertexBuffer.allocator;
        rSubmes.resources[2] = deviceVertexBuffer.buffer;
        return rSubmes;
    }

    void DeviceVK::deleteTexture2D(RTexture2D tex2d)
    {
        MemoryHelperVK::freeImage( *(ImageVK*) &tex2d.resources );
        scoped_lock lk(tex2dMutex);
        Remove_if ( textures2d, [&](auto& t)
        {
            return memcmp( &t, &tex2d, sizeof(t) )==0;
        });
    }

    void DeviceVK::deleteShader(RShader shader)
    {
        vkDestroyShaderModule( logical, (VkShaderModule) shader.resources[0], nullptr );
        scoped_lock lk(shaderMutex);
        Remove_if (shaders, [&](auto& s)
        {
            return memcmp( &s, &shader, sizeof(s) )==0;
        });
    }

    void DeviceVK::deleteSubmesh(RSubmesh submesh)
    {
        assert(false);
    }

}
#endif
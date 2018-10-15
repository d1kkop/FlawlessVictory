#include "PCH.h"
#if FV_VULKAN
#include "RenderManager.h" // For config
#include "HelperVK.h"
#include "DeviceVK.h"
#include "SwapChainVK.h"
#include "../Core/Directories.h"
#include "../Core/LogManager.h"
#include "../Core/Functions.h"

namespace fv
{
    void DeviceVK::release()
    {
        if ( swapChain ) swapChain->release();
        delete swapChain; swapChain = nullptr;
        for ( auto& ri : renderImages ) ri.release();
        for ( auto& fo : frameSyncObjects) fo.release();
        for ( auto& img : textures2d ) vkDestroyImage( logical, img, nullptr );
        for ( auto& shd : shaders ) vkDestroyShaderModule( logical, shd, nullptr );
        for ( auto& buf : buffers) vkDestroyBuffer( logical, buf, nullptr );
        for ( auto& dvm : deviceMemorys) vkFreeMemory( logical, dvm, nullptr );
        for ( auto& kvp : pipelines ) kvp.second.release();
        vkDestroyRenderPass( logical, clearPass, nullptr );
        vkDestroyShaderModule( logical, standardFrag, nullptr );
        vkDestroyShaderModule( logical, standardVert, nullptr );
        vkDestroyCommandPool( logical, commandPool, nullptr );
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

    bool DeviceVK::createCommandPools()
    {
        assert( logical && queueIndices.graphics && commandPool==nullptr );
        if ( !HelperVK::createCommandPool(logical, queueIndices.graphics.value(), commandPool) )
        {
            return false;
        }
        return true;
    }

    bool DeviceVK::createSwapChain(const RenderConfig& rc, VkSurfaceKHR surface)
    {
        assert(logical && physical && surface && !swapChain);
        swapChain = new SwapChainVK;
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
        return true;
    }

    bool DeviceVK::createRenderImages(const struct RenderConfig& rc)
    {
        assert(renderImages.size()==0 && rc.numImages > 0 && rc.numSamples >= 1);
        renderImages.resize(rc.numImages);
        for ( auto& ri : renderImages )
        {
            if ( !ri.createImages( rc ) ) return false;
            if ( !ri.createImageViews( rc ) ) return false;
            if ( !ri.createFrameBuffers( clearPass ) ) return false;
        }
        return true;
    }

    bool DeviceVK::createFrameSyncObjects(const struct RenderConfig& rc)
    {
        assert(frameSyncObjects.size() == 0 && rc.numFramesBehind >= 1 );
        frameSyncObjects.resize( rc.numFramesBehind );
        for ( auto& fso : frameSyncObjects )
        {
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
            Vector<VkVertexInputAttributeDescription> inputAttribs;
            HelperVK::createVertexAttribs(sinput, inputAttribs, vertexSize);

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
            if ( !HelperVK::createPipeline(logical, nullptr, nullptr, nullptr, renderPass, vp, inputAttribs, vertexSize, pipeline, layout) )
            {
                // pipelineMutex is already unlocked
                return false;
            }

            pipelineOut.pipeline;
            pipelineOut.layout = layout;

            pipelines[pipelineHash] = pipelineOut;
        }
        else
        {
            pipelineOut = pIt->second;
        }
        return true;
    }

}
#endif
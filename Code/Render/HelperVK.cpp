#include "HelperVK.h"
#if FV_VULKAN
// -- Change these includes to not need these anymore. HelperVK should only depend on Vulkan, not Higher level Vulkan implementations. --
#include "RenderManager.h" // For RenderConfig
#include "DeviceVK.h"
#include "PipelineVK.h"
// -- End change includes --- 
#include "../Core/Algorithm.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
#include "../Core/IncSDL.h"

namespace fv
{
    bool HelperVK::createInstance(const String& appName,
                                  const Vector<const char*>& requiredExtensions,
                                  const Vector<const char*>& requiredLayers,
                                  VkInstance& instance)
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName.c_str();
        appInfo.applicationVersion = 1;
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = 1;
        appInfo.apiVersion = VK_API_VERSION_1_1; // VK_API_VERSION_1_0

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = (u32)requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = (u32)requiredLayers.size();
        createInfo.ppEnabledLayerNames = requiredLayers.data();

        if ( vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS )
        {
            LOGW("VK Create instance failed.");
            return false;
        }

        return true;
    }

    bool HelperVK::createDebugCallback(VkInstance instance, bool includeVerbose, bool includeInfo, DebugCallbackVK cb, VkDebugUtilsMessengerEXT& debugCallback)
    {
        assert( instance && cb );
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            (includeVerbose?VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:0) |
            (includeInfo?VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:0) |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = cb;
        createInfo.pUserData = nullptr; // Optional
        auto createDebugUtilsMesgenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if ( !createDebugUtilsMesgenger ||
            createDebugUtilsMesgenger(instance, &createInfo, nullptr, &debugCallback) != VK_SUCCESS )
        {
            LOGW("VK Setup debug callback failed.");
            return false;
        }
        return true;
    }
    
    bool HelperVK::createDevice(VkInstance instance, VkPhysicalDevice physical, const Vector<VkDeviceQueueCreateInfo>& queueCreateInfos, 
                                const Vector<const char*>& requiredExtensions, const Vector<const char*>& requiredLayers, VkDevice& logical)
    {
        assert( instance && physical );
        VkPhysicalDeviceFeatures deviceFeatures = {};
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = (u32)queueCreateInfos.size();
        createInfo.pEnabledFeatures  = &deviceFeatures;
        createInfo.enabledExtensionCount = (u32)requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = (u32)requiredLayers.size();
        createInfo.ppEnabledLayerNames = requiredLayers.data();

        auto res = vkCreateDevice(physical, &createInfo, nullptr, &logical);
        if ( res != VK_SUCCESS )
        {
            LOGC("VK Failed to create logical device.");
            return false;
        }
        return true;
    }

    bool HelperVK::createSurface(VkInstance instance, const void* wHandle, VkSurfaceKHR& surface)
    {
        assert( instance);
    #if FV_SDL
        bool bResult = SDL_Vulkan_CreateSurface((SDL_Window*)wHandle, instance, &surface);
        if ( !bResult )
        {
            LOGC("SDL_VK Failed to create vulkan window surface. SDL error %s.", SDL_GetError());
            return false;
        }
        return true;
    #else
    #error no implementation
    #endif
        return false;
    }

    bool HelperVK::createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                   u32 width, u32 height, u32 numImages, u32 numLayers,
                                   const Optional<u32>& graphicsQueueIdx, const Optional<u32>& presentQueueIdx,
                                   VkSurfaceFormatKHR& chosenFormat, VkPresentModeKHR& chosenPresentMode,
                                   VkExtent2D& surfaceExtend, VkSwapchainKHR oldSwapChain, VkSwapchainKHR& swapChain)
    {
        assert(device && physicalDevice && surface && width > 0 && height > 0 && numImages > 0 && numLayers > 0 );

        if ( !(graphicsQueueIdx.has_value() && presentQueueIdx.has_value()) )
        {
            LOG("VK Invalid queue indices to set up swap chain.");
            return false;
        }

        Vector<VkPresentModeKHR> presentModes;
        Vector<VkSurfaceFormatKHR> formats;
        VkSurfaceCapabilitiesKHR capabilities;
        querySwapChainInfo(physicalDevice, surface, formats, capabilities, presentModes);
        if ( !chooseSwapChain(width, height, formats, capabilities, presentModes, chosenFormat, chosenPresentMode, surfaceExtend) )
        {
            return false;
        }

        u32 imageCount = Max<u32>(numImages, capabilities.minImageCount);
        if ( capabilities.maxImageCount != 0 ) // Only clamp to max if specified. Some GPU's do not specify.
        {
            imageCount = Min<u32>(imageCount, capabilities.maxImageCount);
        }
        u32 imageArrayLayerCount = Clamp<u32>(numLayers, 1U, (u32)capabilities.maxImageArrayLayers);

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = chosenFormat.format;
        createInfo.imageColorSpace = chosenFormat.colorSpace;
        createInfo.imageExtent = surfaceExtend;
        createInfo.imageArrayLayers = imageArrayLayerCount; // In case of 3d stereo rendering must be 2
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = { graphicsQueueIdx.value(), presentQueueIdx.value() };
        if ( queueFamilyIndices[0] != queueFamilyIndices[1] )
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = capabilities.currentTransform;   // Pre transform the image (eg flip horizontal)
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // If want to blend with other windows in system
        createInfo.presentMode = chosenPresentMode;
        createInfo.clipped = VK_TRUE; // Whether hidden pixels by other windows are obscured
        createInfo.oldSwapchain = oldSwapChain;

        if ( vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS )
        {
            LOGC("VK Failed to create swap chain for device.");
            return false;
        }
        return true;
    }

    bool HelperVK::createImageView(VkDevice device, VkImage image, VkFormat format, u32 numLayers, VkImageAspectFlags aspectFlags, VkImageView& imgView)
    {
        assert(device && image && format);

        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = aspectFlags;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = numLayers;

        if ( vkCreateImageView(device, &createInfo, nullptr, &imgView) != VK_SUCCESS )
        {
            LOGC("VK failed to create image view.");
            return false;
        }
        return true;
    }

    bool HelperVK::createShaderFromBinary(VkDevice device, const Path& path, VkShaderModule& shaderModule)
    {
        assert(device);
        Vector<char> code;
        if ( !LoadBinaryFile(path.string().c_str(), code) )
        {
            // Allow failing to load
            return false;
        }
        if ( !createShaderModule(device, code, shaderModule) )
        {
            return false;
        }
        return true;
    }

    bool HelperVK::createShaderModule(VkDevice device, const char* data, u32 size, VkShaderModule& shaderModule)
    {
        assert(device);
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = size;
        createInfo.pCode = (const uint32_t*)data;
        if ( vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS )
        {
            LOGC("Failed to create shader module.");
            return false;
        }
        return true;
    }

    bool HelperVK::createShaderModule(VkDevice device, const Vector<char>& code, VkShaderModule& shaderModule)
    {
        return createShaderModule( device, code.data(), (u32)code.size(), shaderModule );
    }

    bool HelperVK::createPipeline(VkDevice device,
                                  const PipelineFormatVK& format,
                                  const VkViewport& vp,
                                  const Vector<VkVertexInputBindingDescription>& vertexBindings,
                                  const Vector<VkVertexInputAttributeDescription>& vertexAttribs,
                                  VkPipeline& pipeline,
                                  VkPipelineLayout& pipelineLayout)
    {
        assert(device && format.mdata.vertShader.resources[0] && format.mdata.fragShader.resources[0] && format.renderPass);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = (VkShaderModule) format.mdata.vertShader.resources[0];
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = (VkShaderModule) format.mdata.fragShader.resources[0];
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo geomShaderStageInfo = {};
        geomShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        geomShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        geomShaderStageInfo.module = (VkShaderModule) format.mdata.geomShader.resources[0];
        geomShaderStageInfo.pName = "main";

        u32 numShaderStages = 2;
        VkPipelineShaderStageCreateInfo shaderStages[8] = { vertShaderStageInfo, fragShaderStageInfo };
        if ( format.mdata.geomShader.resources[0] ) shaderStages[numShaderStages++] = geomShaderStageInfo;

        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = format.vertexSize;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = (u32)vertexBindings.size();
        vertexInputInfo.pVertexBindingDescriptions = vertexBindings.data();
        vertexInputInfo.vertexAttributeDescriptionCount = (u32)vertexAttribs.size();
        vertexInputInfo.pVertexAttributeDescriptions = vertexAttribs.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkRect2D scissor;
        scissor.offset = { 0, 0 };
        scissor.extent = { (u32)vp.width, (u32)vp.height };

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &vp;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = format.polyMode;
        rasterizer.lineWidth = format.lineWidth;
        rasterizer.cullMode  = format.cullmode;
        rasterizer.frontFace = format.frontFace;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = format.depthTest;
        depthStencil.depthWriteEnable = format.depthWrite;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = format.stencilTest;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = (VkSampleCountFlagBits)format.numSamples;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if ( vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS )
        {
            LOGC("VK Failed to create base pipeline layout");
            return false;
        }

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = numShaderStages;
        pipelineInfo.pStages = shaderStages; // vertex and fragment (minimum)
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = format.renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if ( vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS )
        {
            LOGC("VK Failed to create base graphics pipeline.");
            vkDestroyPipelineLayout( device, pipelineLayout, nullptr );
            pipelineLayout = nullptr;
            return false;
        }

        return true;
    }

    bool HelperVK::createFramebuffer(VkDevice device, const VkExtent2D& size, VkRenderPass renderPass, 
                                     const Vector<VkImageView>& attachments, VkFramebuffer& framebuffer)
    {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = (u32)attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width  = size.width;
        framebufferInfo.height = size.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) 
        {
            LOGC("VK Failed to create framebuffer.");
            return false;
        }
        return true;
    }

    bool HelperVK::createCommandPool(VkDevice device, u32 familyQueueIndex, VkCommandPool& commandPool)
    {
        assert( device );
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = familyQueueIndex;
        if ( vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS )
        {
            LOGC("VK Failed to create command pool.");
            return false;
        }
        return true;
    }

    void HelperVK::createVertexAttribs(const SubmeshInput& s, Vector<VkVertexInputAttributeDescription>& inputAttribs, u32& vertexSize)
    {
        auto& inputs = inputAttribs;
        inputs.clear();
        u32 vertexComponentCount = 0;
        VkVertexInputAttributeDescription input;

        // Vertex
        if ( s.positions )
        {
            input.binding  = 0;
            input.location = VertexType::Position;
            input.offset = 0;
            input.format = VK_FORMAT_R32G32B32_SFLOAT;
            inputs.emplace_back(input);
            vertexComponentCount += 3;
        }
        if ( s.normals )
        {
            input.binding  = 0;
            input.location = VertexType::Normal;
            input.offset = vertexComponentCount*sizeof(float);
            input.format = VK_FORMAT_R32G32B32_SFLOAT;
            inputs.emplace_back(input);
            vertexComponentCount += 3;
        }
        if ( s.tanBins )
        {
            // tangent
            input.binding  = 0;
            input.location = VertexType::Tangent;
            input.offset = vertexComponentCount*sizeof(float);
            input.format = VK_FORMAT_R32G32B32_SFLOAT;
            inputs.emplace_back(input);
            vertexComponentCount += 3;
            // bitangent
            input.binding  = 0;
            input.location = VertexType::Bitangent;
            input.offset = vertexComponentCount*sizeof(float);
            input.format = VK_FORMAT_R32G32B32_SFLOAT;
            inputs.emplace_back(input);
            vertexComponentCount += 3;
        }
        if ( s.uvs )
        {
            input.binding  = 0;
            input.location = VertexType::Uv;
            input.offset = vertexComponentCount*sizeof(float);
            input.format = VK_FORMAT_R32G32_SFLOAT;
            inputs.emplace_back(input);
            vertexComponentCount += 2;
        }
        if ( s.lightUvs )
        {
            input.binding  = 0;
            input.location = VertexType::LightUv;
            input.offset = vertexComponentCount*sizeof(float);
            input.format = VK_FORMAT_R32G32_SFLOAT;
            inputs.emplace_back(input);
            vertexComponentCount += 2;
        }
        for ( u32 i=0; i<4; ++i )
        {
            if ( s.extras[i] )
            {
                input.binding  = 0;
                input.location = VertexType::Extra1 + i;
                input.offset = vertexComponentCount*sizeof(float);
                input.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                inputs.emplace_back(input);
                vertexComponentCount += 4;
            }
        }
        if ( s.bones )
        {
            // weights
            input.binding  = 0;
            input.location = VertexType::Weights;
            input.offset = vertexComponentCount*sizeof(float);
            input.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            inputs.emplace_back(input);
            vertexComponentCount += 4;
            // bone indices
            input.binding  = 0;
            input.location = VertexType::BoneIndices;
            input.offset = vertexComponentCount*sizeof(float);
            input.format = VK_FORMAT_R32_UINT;
            inputs.emplace_back(input);
            vertexComponentCount += 1;
        }

        // This assumes that u32 is equal to float32
        static_assert( sizeof(float) == sizeof(u32) ); 
        vertexSize = vertexComponentCount*sizeof(float);
    }

    void HelperVK::allocCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer& commandBuffer)
    {
        assert(device && commandPool);
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        FV_VKCALL ( vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) );
    }

    void HelperVK::allocCommandBuffers(VkDevice device, VkCommandPool commandPool, u32 numCommandBuffers, Vector<VkCommandBuffer>& commandBuffers)
    {
        assert(device && commandPool);
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = numCommandBuffers;
        u32 oldSize = (u32)commandBuffers.size();
        commandBuffers.resize(commandBuffers.size() + numCommandBuffers);
        FV_VKCALL( vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data() + oldSize) );
    }

    void HelperVK::beginCommandBuffer(VkDevice device, VkCommandBufferUsageFlags usage, VkCommandBuffer commandBuffer)
    {
        assert( device && commandBuffer );
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = usage;
        FV_VKCALL( vkBeginCommandBuffer(commandBuffer, &beginInfo) );
    }

    void HelperVK::startRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer frameBuffer, const VkRect2D& renderArea, 
                                   const VkClearValue* clearValue, u32 numClearValues)
    {
        assert( commandBuffer && renderPass && frameBuffer );
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass  = renderPass;
        renderPassInfo.framebuffer = frameBuffer;
        renderPassInfo.renderArea.offset = renderArea.offset;
        renderPassInfo.renderArea.extent = renderArea.extent;
        renderPassInfo.clearValueCount = numClearValues;
        renderPassInfo.pClearValues = clearValue;
        vkCmdBeginRenderPass( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    }

    void HelperVK::stopRenderPass(VkCommandBuffer commandBuffer)
    {
        assert( commandBuffer );
        vkCmdEndRenderPass( commandBuffer );
    }

    void HelperVK::endCommandBuffer(VkCommandBuffer commandBuffer)
    {
        assert( commandBuffer );
        FV_VKCALL( vkEndCommandBuffer(commandBuffer) );
    }

    void HelperVK::freeCommandBuffers(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* buffers, u32 numBuffers)
    {
        assert( device && commandPool );
        if (!buffers ||numBuffers==0) return;
        vkFreeCommandBuffers( device, commandPool, numBuffers, buffers );
    }

    void HelperVK::queryRequiredWindowsExtensions(void* pWindow, Vector<const char*>& extensions)
    {
        assert( pWindow );
    #if FV_SDL
        uint32_t extension_count;
        SDL_Vulkan_GetInstanceExtensions((SDL_Window*)pWindow, &extension_count, nullptr);
        auto oldSize = extensions.size();
        extensions.resize(extensions.size() + extension_count);
        SDL_Vulkan_GetInstanceExtensions((SDL_Window*)pWindow, &extension_count, &extensions[oldSize]);
    #else
    #error no implementation
    #endif
    }

    bool HelperVK::checkRequiredExtensions(const Vector<const char*>& requiredList, VkPhysicalDevice physicalDevice)
    {
        Vector<String> foundExtensions;
        queryExtensions(foundExtensions, physicalDevice);
        return validateNameList(foundExtensions, requiredList);
    }

    bool HelperVK::checkRequiredLayers(const Vector<const char*>& requiredList, VkPhysicalDevice physicalDevice)
    {
        Vector<String> foundLayers;
        queryLayers(foundLayers, physicalDevice);
        return validateNameList(foundLayers, requiredList);
    }

    void HelperVK::queryExtensions(Vector<String>& foundExtensions, VkPhysicalDevice physicalDevice)
    {
        uint32_t extensionCount = 0;
        Vector<VkExtensionProperties> extensions;
        if ( !physicalDevice ) vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        else vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
        extensions.resize(extensionCount);
        if ( !physicalDevice ) vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        else vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());
        for ( u32 i=0; i< extensionCount; ++i ) foundExtensions.emplace_back(extensions[i].extensionName);
    }

    void HelperVK::queryLayers(Vector<String>& foundLayers, VkPhysicalDevice physicalDevice)
    {
        uint32_t layerCount;
        Vector<VkLayerProperties> layers;
        if ( !physicalDevice ) vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        else vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, nullptr);
        layers.resize(layerCount);
        if ( !physicalDevice ) vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
        else vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, layers.data());
        for ( u32 i = 0; i < layerCount; i++ ) foundLayers.emplace_back(layers[i].layerName);
    }

    bool HelperVK::validateNameList(const Vector<String>& found, const Vector<const char*>& required)
    {
        for ( auto* e : required )
        {
            if ( !Contains(found, e) )
            {
                LOGW("VK Cannot find requested %s.", e);
                return false;
            }
        }
        return true;
    }

    void HelperVK::getQueueIndices(VkPhysicalDevice physical, VkSurfaceKHR surface, QueueFamilyIndicesVK& queueIndices)
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

    bool HelperVK::chooseSwapChain(u32 width, u32 height, 
                                   const Vector<VkSurfaceFormatKHR>& formats, 
                                   const VkSurfaceCapabilitiesKHR& capabilities, 
                                   const Vector<VkPresentModeKHR>& presentModes,
                                   VkSurfaceFormatKHR& format, VkPresentModeKHR& mode, VkExtent2D& extent)
    {
        bool found = false;
        if ( formats.size() > 0 )
        {
            if ( formats[0].format == VK_FORMAT_UNDEFINED )
            {
                format = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
                found = true;
            }
            else
            {
                for ( const auto& f : formats )
                {
                    if ( f.format == VK_FORMAT_B8G8R8A8_UNORM  && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
                    {
                        format = f;
                        found = true;
                        break;
                    }
                }
            }
        }
        if ( !found )
        {
            return false;
        }
        mode = VK_PRESENT_MODE_FIFO_KHR;
        for ( const auto& presentMode : presentModes )
        {
            if ( presentMode == VK_PRESENT_MODE_MAILBOX_KHR )
            {
                mode = presentMode;
                break;
            }
            else if ( presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR )
            {
                mode = presentMode;
            }
        }
        if ( capabilities.currentExtent.width != UINT_MAX )
        {
            extent = capabilities.currentExtent;
        }
        else
        {
            extent = { width, height };
            extent.width  = Clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = Clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }
        if ( extent.width < 1 || extent.height < 1 )
        {
            return false;
        }
        return true;
    }

    void HelperVK::querySwapChainInfo(VkPhysicalDevice device, VkSurfaceKHR surface, 
                                      Vector<VkSurfaceFormatKHR>& formats,
                                      VkSurfaceCapabilitiesKHR& capabilities, 
                                      Vector<VkPresentModeKHR>& presentModes)
    {
        assert( device && surface );

        uint32_t formatCount;
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if ( formatCount > 0 )
        {
            formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());
        }
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if ( presentModeCount > 0 )
        {
            presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());
        }
    }

    u32 HelperVK::findMemoryType(u32 typeFilter, const VkPhysicalDeviceMemoryProperties& memProperties, VkMemoryPropertyFlags propertyFlags)
    {
        for ( u32 i = 0; i < memProperties.memoryTypeCount; i++ )
        {
            if ( (typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags )
            {
                return i;
            }
        }
        return -1;
    }

    VkFormat HelperVK::convert(ImageFormat imgFormat)
    {
        switch ( imgFormat )
        {
        case ImageFormat::RGBA8:
            return VK_FORMAT_R8G8B8A8_SRGB;

        case ImageFormat::RGB8:
            return VK_FORMAT_R8G8B8_SRGB;

        case ImageFormat::Single8:
            return VK_FORMAT_R8_SRGB;

        case ImageFormat::RGBA16:
            return VK_FORMAT_R16G16B16A16_UNORM;

        case ImageFormat::RGB16:
            return VK_FORMAT_R16G16B16_UNORM;

        case ImageFormat::RGBA32F:
            return VK_FORMAT_R32G32B32A32_SFLOAT;

        case ImageFormat::RGB32F:
            return VK_FORMAT_R32G32B32_SFLOAT;

        case ImageFormat::RGB555:
            return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
        }
        return VK_FORMAT_UNDEFINED;
    }

    VkFormat HelperVK::findSupportedFormat(VkPhysicalDevice device, const Vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        assert( device );
        for ( VkFormat format : candidates )
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(device, format, &props);
            if ( tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features )
            {
                return format;
            }
            else if ( tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features )
            {
                return format;
            }
        }
        return VK_FORMAT_UNDEFINED;
    }

    VkFormat HelperVK::findDepthFormat(VkPhysicalDevice device)
    {
        return findSupportedFormat(device, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

}

#endif
#include "HelperVK.h"
#if FV_VULKAN
#include "RenderManager.h" // For RenderConfig
#include "../Core/Algorithm.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"

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
            LOGW("VK Failed to create logical device.");
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
            LOGW("SDL_VK Failed to create vulkan window surface. SDL error %s.", SDL_GetError());
            return false;
        }
        return true;
    #else
    #error no implementation
    #endif
        return false;
    }

    bool HelperVK::createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                   const RenderConfig& rc, const Optional<u32>& graphicsQueueIdx, const Optional<u32>& presentQueueIdx,
                                   VkSurfaceFormatKHR& chosenFormat, VkPresentModeKHR& chosenPresentMode,
                                   VkExtent2D& surfaceExtend, VkSwapchainKHR& swapChain)
    {
        assert(device && physicalDevice && surface && rc.windowWidth != 0 && 
               rc.windowHeight != 0 && rc.numImages >= 1 && rc.numLayers >= 1 && rc.numSamples >= 1);

        if ( !(graphicsQueueIdx.has_value() && presentQueueIdx.has_value()) )
        {
            return false;
        }

        Vector<VkPresentModeKHR> presentModes;
        Vector<VkSurfaceFormatKHR> formats;
        VkSurfaceCapabilitiesKHR capabilities;
        querySwapChainInfo(physicalDevice, surface, formats, capabilities, presentModes);
        if ( !chooseSwapChain(rc.windowWidth, rc.windowHeight, formats, capabilities, presentModes, chosenFormat, chosenPresentMode, surfaceExtend) )
        {
            return false;
        }

        u32 imageCount = Max<u32>(rc.numImages, capabilities.minImageCount);
        if ( capabilities.maxImageCount != 0 ) // Only clamp to max if specified. Some GPU's do not specify.
        {
            imageCount = Min<u32>(imageCount, capabilities.maxImageCount);
        }
        u32 imageArrayLayerCount = Clamp<u32>(rc.numLayers, 1U, (u32)capabilities.maxImageArrayLayers);

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
        createInfo.oldSwapchain = VK_NULL_HANDLE; // TODO fix later

        if ( vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS )
        {
            LOGW("VK Failed to create swap chain for device.");
            return false;
        }
        return true;
    }

    bool HelperVK::createImageView(VkDevice device, VkImage image, VkFormat format, u32 numLayers, VkImageView& imgView)
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

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
            LOGW("VK Faild to load shader %s.", path.string().c_str());
            return false;
        }
        if ( !createShaderModule(device, code, shaderModule) )
        {
            LOGW("VK failed to create shader module for %s.", path.string().c_str());
            return false;
        }
        return true;
    }

    bool HelperVK::createShaderModule(VkDevice device, const Vector<char>& code, VkShaderModule& shaderModule)
    {
        assert(device);
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = (const uint32_t*)code.data();
        if ( vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS )
        {
            LOGW("Failed to create shader module.");
            return false;
        }
        return true;
    }

    bool HelperVK::createRenderPass(VkDevice device, VkFormat colorFormat, u32 samples, VkRenderPass& renderPass)
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format  = colorFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if ( vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS )
        {
            LOGW("VK Failed to create render pass.");
            return false;
        }
        return true;
    }

    bool HelperVK::createPipeline(VkDevice device,
                                  VkShaderModule vertShader,
                                  VkShaderModule fragShader,
                                  VkShaderModule geomShader,
                                  VkRenderPass renderPass,
                                  const VkViewport& vp,
                                  const Vector<VkVertexInputAttributeDescription>& vertexInputs,
                                  u32 totalVertexSize,
                                  VkPipeline& pipeline,
                                  VkPipelineLayout& pipelineLayout)
    {
        assert(device && vertShader && fragShader && renderPass);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShader;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShader;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo geomShaderStageInfo = {};
        geomShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        geomShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        geomShaderStageInfo.module = geomShader;
        geomShaderStageInfo.pName = "main";

        u32 numShaderStages = 2;
        VkPipelineShaderStageCreateInfo shaderStages[8] = { vertShaderStageInfo, fragShaderStageInfo };
        if ( geomShader ) shaderStages[numShaderStages++] = geomShaderStageInfo;

        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = totalVertexSize;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = (u32)vertexInputs.size();
        vertexInputInfo.pVertexAttributeDescriptions = vertexInputs.data();

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
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

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
            LOGW("VK Failed to create base pipeline layout");
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
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if ( vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS )
        {
            LOGW("VK Failed to create base graphics pipeline.");
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
            LOGW("VK Failed to create framebuffer.");
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
            LOGW("VK Failed to create command pool.");
            return false;
        }
        return true;
    }

    bool HelperVK::createVertexBuffer(VkDevice device, const VkPhysicalDeviceMemoryProperties& memProperties,
                                      const void* data, u32 bufferSize, bool shareInQueues, bool coherentMemory, 
                                      VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory)
    {
        assert( device && data && bufferSize != 0 );

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = shareInQueues ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

        if ( vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS )
        {
            LOGW("VK Failed to create vertex buffer.");
            return false;
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        u32 flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        flags |= coherentMemory ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : 0;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, memProperties, flags);

        if ( vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS )
        {
            LOGW("VK Failed to allocate memory for vertex buffer.");
            return false;
        }

        vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

        void* mappedData;
        if ( vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &mappedData) == VK_SUCCESS )
        {
            memcpy(mappedData, data, bufferSize);
            vkUnmapMemory(device, vertexBufferMemory);
        }
        else 
        {
            LOGW("VK Failed to map and update vertex buffer.");
            return false;
        }

        return true;
    }

    void HelperVK::createVertexAttribs(const SubmeshInput& s, Vector<VkVertexInputAttributeDescription>& inputAttribs, u32& vertexSize)
    {
        auto& inputs = inputAttribs;
        inputs.clear();
        inputs.reserve(VertexType::COUNT);
        u32 vertexComponentCount = 0;
        VkVertexInputAttributeDescription input;

        // Vertex
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

    bool HelperVK::allocCommandBuffers(VkDevice device, VkCommandPool commandPool, u32 numCommandBuffers, Vector<VkCommandBuffer>& commandBuffers)
    {
        assert(device && commandPool);
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = numCommandBuffers;
        u32 oldSize = (u32)commandBuffers.size();
        commandBuffers.resize(commandBuffers.size() + numCommandBuffers);
        if ( vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data() + oldSize) != VK_SUCCESS )
        {
            LOGW("VK Failed to allocate command buffer.");
            return false;
        }
        return true;
    }

    bool HelperVK::startRecordCommandBuffer(VkDevice device, VkCommandBuffer commandBuffer)
    {
        assert( device && commandBuffer );

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        if ( vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS )
        {
            LOGW("VK Failed to begin command buffer.");
            return false;
        }

        return true;
    }

    void HelperVK::startRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer frameBuffer, const VkRect2D& renderArea, const VkClearValue* clearVal)
    {
        assert( commandBuffer && renderPass && frameBuffer );
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass  = renderPass;
        renderPassInfo.framebuffer = frameBuffer;
        renderPassInfo.renderArea.offset = renderArea.offset;
        renderPassInfo.renderArea.extent = renderArea.extent;

        renderPassInfo.clearValueCount = clearVal ? 1 : 0;
        renderPassInfo.pClearValues = clearVal;

        vkCmdBeginRenderPass( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    }

    void HelperVK::stopRenderPass(VkCommandBuffer commandBuffer)
    {
        assert( commandBuffer );
        vkCmdEndRenderPass( commandBuffer );
    }

    bool HelperVK::stopRecordCommandBuffer(VkCommandBuffer commandBuffer)
    {
        assert( commandBuffer );
        if ( vkEndCommandBuffer(commandBuffer) != VK_SUCCESS )
        {
            LOGW("VK Failed to end command buffer.");
            return false;
        }
        return true;
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

    bool HelperVK::chooseSwapChain(u32 width, u32 height, 
                                   const Vector<VkSurfaceFormatKHR>& formats, 
                                   const VkSurfaceCapabilitiesKHR& capabilities, 
                                   const Vector<VkPresentModeKHR>& presentModes,
                                   VkSurfaceFormatKHR& format, VkPresentModeKHR& mode, VkExtent2D& extend)
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
            LOGW("VK Could not find suitable surface format for swap chain.");
            return false;
        }
        mode  = VK_PRESENT_MODE_FIFO_KHR;
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
            extend = capabilities.currentExtent;
        }
        else
        {
            extend = { width, height };
            extend.width  = Clamp(extend.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extend.height = Clamp(extend.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
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

    void HelperVK::storeDeviceQueueFamilies(DeviceVK& device, VkSurfaceKHR surface)
    {
        assert(device.physical);
        uint32_t queueFamilyCount;
        Vector<VkQueueFamilyProperties> queueFamilies;
        vkGetPhysicalDeviceQueueFamilyProperties(device.physical, &queueFamilyCount, nullptr);
        queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device.physical, &queueFamilyCount, queueFamilies.data());
        for ( u32 i=0; i<queueFamilyCount; ++i )
        {
            auto& queueFam = queueFamilies[i];
            if ( queueFam.queueCount > 0 )
            {
                if ( (queueFam.queueFlags & VK_QUEUE_GRAPHICS_BIT) ) device.queueIndices.graphics = i;
                if ( (queueFam.queueFlags & VK_QUEUE_COMPUTE_BIT) )  device.queueIndices.compute = i;
                if ( (queueFam.queueFlags & VK_QUEUE_TRANSFER_BIT) ) device.queueIndices.transfer = i;
                if ( (queueFam.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ) device.queueIndices.sparse = i;
            }
            VkBool32 presentSupported = false;
            if ( surface )
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(device.physical, i, surface, &presentSupported);
                if ( presentSupported ) device.queueIndices.present = i;
            }
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

}

#endif
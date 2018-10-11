#include "HelperVK.h"
#if FV_VULKAN
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

    bool HelperVK::createSwapChain(const SwapChainParamsVK& p, SwapChainVK& swapChain)
    {
        assert(p.device && p.surface && p.width != 0 && p.height != 0);

        SwapChainInfoVK chainInfo;
        querySwapChainInfo(p.device->physical, p.surface, chainInfo);

        if ( !chooseSwapChain(p.width, p.height, chainInfo, swapChain.surfaceFormat, swapChain.presentMode, swapChain.extend) )
        {
            return false;
        }

        u32 imageCount = Max<u32>(p.imageCount, chainInfo.capabilities.minImageCount);
        if ( chainInfo.capabilities.maxImageCount != 0 ) // Only clamp to max if specified. Some GPU's do not specify.
        {
            imageCount = Min<u32>(imageCount, chainInfo.capabilities.maxImageCount);
        }
        u32 imageArrayLayers = Clamp<u32>(p.imageArrayLayerCount, 1U, (u32)chainInfo.capabilities.maxImageArrayLayers);

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = p.surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = swapChain.surfaceFormat.format;
        createInfo.imageColorSpace = swapChain.surfaceFormat.colorSpace;
        createInfo.imageExtent = swapChain.extend;
        createInfo.imageArrayLayers = imageArrayLayers; // In case of 3d stereo rendering must be 2
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        assert(p.device->queueIndices.graphics.has_value() && p.device->queueIndices.present.has_value());
        uint32_t queueFamilyIndices[] = { p.device->queueIndices.graphics.value(), p.device->queueIndices.present.value() };
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

        createInfo.preTransform = chainInfo.capabilities.currentTransform;   // Pre transform the image (eg flip horizontal)
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // If want to blend with other windows in system
        createInfo.presentMode = swapChain.presentMode;
        createInfo.clipped = VK_TRUE; // Whether hidden pixels by other windows are obscured
        createInfo.oldSwapchain = VK_NULL_HANDLE; // TODO fix later

        if ( vkCreateSwapchainKHR(p.device->logical, &createInfo, nullptr, &swapChain.swapChain) != VK_SUCCESS )
        {
            LOGC("VK Failed to create swap chain for device.");
            return false;
        }

        // Retrieve swap chain images
        assert(swapChain.swapChain);
        uint32_t swapChainImgCount;
        vkGetSwapchainImagesKHR(p.device->logical, swapChain.swapChain, &swapChainImgCount, nullptr);
        swapChain.images.resize(swapChainImgCount);
        vkGetSwapchainImagesKHR(p.device->logical, swapChain.swapChain, &swapChainImgCount, swapChain.images.data());

        // Create image views on images in swap chain
        for ( auto& img : swapChain.images )
        {
            VkImageView imgView;
            if ( !createImageView(p.device->logical, img, swapChain.surfaceFormat.format, imgView) )
            {
                LOGC("VK Cannot create all images for swap chain. Render setup failed.");
                return false;
            }
            swapChain.imgViews.emplace_back(imgView);
        }

        // For each image, there is a framebuffer and ascociated img views.
        for ( auto& img : swapChain.images )
        {
            VkFramebuffer frameBuffer;
            if ( !createFramebuffer(p.device->logical, swapChain.extend, p.renderPass, swapChain.imgViews, frameBuffer) )
            {
                LOGC("VK Cannot create all frame buffers for swap chain. Render setup failed.");
                return false;
            }
            swapChain.frameBuffers.emplace_back(frameBuffer);
        }

        swapChain.surface = p.surface;
        swapChain.device = p.device;

        assert(swapChain.imgViews.size() == swapChain.images.size());
        return true;
    }

    bool HelperVK::createImageView(VkDevice device, VkImage image, VkFormat format, VkImageView& imgView)
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
        createInfo.subresourceRange.layerCount = 1;

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

    bool HelperVK::createRenderPass(VkDevice device, VkFormat colorFormat, VkRenderPass& renderPass)
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
                                  VkRenderPass renderPass,
                                  VkExtent2D vpSize,
                                  VkPipelineLayout& pipelineLayout,
                                  VkPipeline& pipeline)
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

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)vpSize.width;
        viewport.height = (float)vpSize.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = vpSize;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
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
            LOGC("VK Failed to create base pipeline layout");
            return false;
        }

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
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
            LOGC("VK Failed to create base graphics pipeline.");
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

    bool HelperVK::allocCommandBuffers(VkDevice device, VkCommandPool commandPool, u32 numCommandBuffers, Vector<VkCommandBuffer>& commandBuffers)
    {
        assert(device && commandPool);
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = numCommandBuffers;
        commandBuffers.resize(numCommandBuffers);
        if ( vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS )
        {
            LOGW("VK Failed to allocate command buffer.");
            return false;
        }
        return true;
    }

    bool HelperVK::startRecordCommandBuffer(VkDevice device, VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer frameBuffer,
                                            const VkRect2D& renderArea, const VkClearValue* clearValue)
    {
        assert( device && commandBuffer && renderPass && frameBuffer );

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass  = renderPass;
        renderPassInfo.framebuffer = frameBuffer;
        renderPassInfo.renderArea.offset = renderArea.offset;
        renderPassInfo.renderArea.extent = renderArea.extent;

        renderPassInfo.clearValueCount = clearValue ? 1 : 0;
        renderPassInfo.pClearValues = clearValue;

        if ( vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS )
        {
            LOGW("VK Failed to begin command buffer.");
            return false;
        }

        return true;
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

    bool HelperVK::chooseSwapChain(u32 width, u32 height, const SwapChainInfoVK& info,
                                   VkSurfaceFormatKHR& format, VkPresentModeKHR& mode, VkExtent2D& extend)
    {
        bool found = false;
        if ( info.formats.size() > 0 )
        {
            if ( info.formats[0].format == VK_FORMAT_UNDEFINED )
            {
                format = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
                found = true;
            }
            else
            {
                for ( const auto& f : info.formats )
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
            LOGC("VK Could not find suitable surface format for swap chain.");
            return false;
        }
        mode  = VK_PRESENT_MODE_FIFO_KHR;
        for ( const auto& presentMode : info.presentModes )
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
        if ( info.capabilities.currentExtent.width != UINT_MAX )
        {
            extend = info.capabilities.currentExtent;
        }
        else
        {
            extend = { width, height };
            extend.width  = Clamp(extend.width, info.capabilities.minImageExtent.width, info.capabilities.maxImageExtent.width);
            extend.height = Clamp(extend.height, info.capabilities.minImageExtent.height, info.capabilities.maxImageExtent.height);
        }
        return true;
    }

    void HelperVK::querySwapChainInfo(VkPhysicalDevice device, VkSurfaceKHR surface, SwapChainInfoVK& info)
    {
        uint32_t formatCount;
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &info.capabilities);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if ( formatCount > 0 )
        {
            info.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, info.formats.data());
        }
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if ( presentModeCount > 0 )
        {
            info.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, info.presentModes.data());
        }
    }

    // TODO these two store functions must be rearranged and not store direclty in DeviceVK.
    void HelperVK::storeDevicePropertiesAndFeatures(DeviceVK& device)
    {
        assert(device.physical);
        vkGetPhysicalDeviceProperties(device.physical, &device.properties);
        vkGetPhysicalDeviceFeatures(device.physical, &device.features);
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
}

#endif
//#include "../../Core/PCH.h"
#include "SimpleRendererVK.h"
#if FV_VULKAN
#include "../../Core/Functions.h"
#include "../../Core/LogManager.h"
#include "../../Core/OSLayer.h"
#include "../../Core/Thread.h"
#include "../../Core/IncGLFW.h"
#include "../../Core/Directories.h"
#include "InstanceVK.h"
#include "DeviceVK.h"
#include "SurfaceVK.h"
#include "SwapChainVK.h"
#include "AllocatorVK.h"
#include "ShaderVK.h"
#include "FenceVK.h"
#include "RenderPassVK.h"
#include "PipelineLayoutVK.h"
#include "PipelineVK.h"
#include "FrameBufferVK.h"
#include "ImageViewVK.h"
#include "CommandPoolVK.h"
#include "CommandBufferVK.h"
#include "SemaphoreVK.h"
#include "BufferVK.h"
#include "HelperVK.h"
#include "QueueVK.h"

namespace fv
{
    SimpleRendererVK::SimpleRendererVK()
    = default;

    SimpleRendererVK::~SimpleRendererVK()
    {
        closeGraphics();
    }

    FV_BG bool SimpleRendererVK::initGraphics()
    {
        FV_CHECK_BG();

        if (!createWindow()) return false;
        if (!createInstance()) return false;
        if (!createSurface()) return false;
        if (!createDevice()) return false;
        if (!createSwapChain()) return false;
        if (!createAllocator()) return false;
        if (!createCommandPool()) return false;
        if (!createShaders()) return false;
        if (!createSimplePass()) return false;
        if (!createPipelineLayout()) return false;
        if (!createPipeline()) return false;
        if (!createFramebuffers()) return false;
        if (!createFrameSyncObjects()) return false;

        if (!createTriangleVertexBuffer()) return false;
        if (!createTriangleCommanBuffer()) return false;

        LOG( "VK Initialized succesful." );
        return true;
    }

    void SimpleRendererVK::closeGraphics()
    {
        if ( m_Device )
        {
            vkDeviceWaitIdle( m_Device->logical() );
        }
        destroyWindow();
    }

    M<DeviceResource> SimpleRendererVK::createShader( const char* code, u32 size )
    {
        M<ShaderVK> shader = ShaderVK::create( m_Device, code, size );
        return {};
    }

    bool SimpleRendererVK::createInstance()
    {
        Vector<const char*> requiredExtensions;
        Vector<const char*> requiredLayers;

    #if FV_DEBUG
        requiredExtensions ={ "VK_EXT_debug_report", "VK_EXT_debug_utils" };
        requiredLayers ={ "VK_LAYER_LUNARG_standard_validation" };
    #endif

    #if FV_GLFW
        // Need these extensions for surface.
        const char** ext;
        uint32_t count;
        ext = glfwGetRequiredInstanceExtensions(&count);
        for ( uint32_t t = 0; t < count; t++) requiredExtensions.emplace_back( ext[t] );
    #endif

        m_Instance = InstanceVK::create( "SimpleVKRenderer", requiredExtensions, requiredLayers );
        if ( m_Instance )
        {
        #if FV_DEBUG
            m_Instance->createDebugCallback( false, false, SimpleRendererVK::debugCallback );
        #endif
            return true;
        }

        return false;
    }

    bool SimpleRendererVK::createSurface()
    {
        m_Surface = SurfaceVK::create( m_Instance, m_Window );
        return m_Surface != nullptr;
    }

    bool SimpleRendererVK::createDevice()
    {
        Vector<const char*> requiredExtensions;
        Vector<const char*> requiredLayers;
        
    #if FV_DEBUG
        requiredLayers ={ "VK_LAYER_LUNARG_standard_validation" };
    #endif
        requiredExtensions ={ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        m_Device = DeviceVK::create( m_Instance, requiredExtensions, requiredLayers, true, true, true, 2, true, true, true, m_Surface );
        if ( m_Device != nullptr )
        {
            m_GraphicsQueue = m_Device->graphicsQueue();
            m_PresentQueue  = m_Device->presentQueue();
            m_TransferQueue = m_Device->transferQueue();
            return m_GraphicsQueue && m_PresentQueue && m_TransferQueue;
        }
        return false;
    }

    bool SimpleRendererVK::createSwapChain()
    {
        u32 width;
        u32 height;
        if ( !OSGetWindowSurfaceSize( m_Window, width, height ))
        {
            LOGC( "VK Failed to obtain window surface size." );
            return false;
        }
        auto queueIndices = m_Device->composeQueueFamIndices( true, false, false, true, false );
        m_SwapChain = SwapChainVK::create( m_Device, m_Surface, width, height, 2, 1, queueIndices );
        return m_SwapChain != nullptr;
    }

    bool SimpleRendererVK::createAllocator()
    {
        m_Allocator = AllocatorVK::create( m_Device );
        return m_Allocator != nullptr;
    }

    bool SimpleRendererVK::createCommandPool()
    {
        m_CommandPool = CommandPoolVK::create( m_Device, m_Device->graphicsQueueFamily() );
        return m_CommandPool != nullptr;
    }

    bool SimpleRendererVK::createShaders()
    {
        Vector<char> code;
        if (!LoadBinaryFile((Directories::standard() / "standard.frag.spv").string().c_str(), code))
        {
            LOGC( "VK Failed to load standard.frag.spv, shader initialization failed." );
            return false;
        }
        m_FragmentShaderSimple = ShaderVK::create( m_Device, code );
        if ( !LoadBinaryFile( (Directories::standard() / "standard.vert.spv").string().c_str(), code ))
        {
            LOGC( "VK Failed to load standard.vert.spv, shader initialization failed." );
            return false;
        }
        m_VertexShaderSimple = ShaderVK::create( m_Device, code );
        if ( !(m_FragmentShaderSimple && m_VertexShaderSimple) )
        {
            LOGC( "VK Failed to load standard frag and vert shader." );
            return false;
        }
        return true;
    }

    bool SimpleRendererVK::createSimplePass()
    {
        m_SimplePlass = RenderPassVK::create( m_Device, m_SwapChain->format().format, 1, AttachmentLoadOp::Clear, AttachmentSaveOp::Save, AttachmentLoadOp::DontCare, AttachmentSaveOp::DontCare );
        return m_SimplePlass != NULL;
    }

    bool SimpleRendererVK::createPipelineLayout()
    {
        m_EmptyPipelineLayout = PipelineLayoutVK::create( m_Device );
        return m_EmptyPipelineLayout != nullptr;
    }

    bool SimpleRendererVK::createPipeline()
    {
        VertexDescriptorSetVK vset;
        vset.addBinding( 0, sizeof(float)*6 );
        vset.addAttrib( 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 );
  //     vset.addAttrib( 0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float)*3 );

        VkViewport vp = HelperVK::makeSimpleViewport( m_SwapChain );

        m_SimplePipeline = PipelineVK::create( m_SimplePlass,
                                               m_EmptyPipelineLayout,
                                               PrimitiveTypeVK::TriangleList,
                                               m_VertexShaderSimple, m_FragmentShaderSimple, NULL, NULL, NULL,
                                               vset, 
                                               vp,
                                               false, false, false,
                                               CullModeVK::None, PolygonModeVK::Fill, FrontFaceVK::CCW, 
                                               1, 1 );
                                               
       return m_SimplePipeline != nullptr;
    }

    bool SimpleRendererVK::createFramebuffers()
    {
        for ( u32 i=0; i < m_SwapChain->numImages(); i++ )
        {
            M<ImageViewVK> imgView = ImageViewVK::create( m_Device, m_SwapChain->image( i ), m_SwapChain->format().format, ViewTypeVK::Type2D, AspectTypeVK::Color, 0, 1, 0, 1 );
            M<FrameBufferVK> frameBuffer = FrameBufferVK::create( m_SimplePlass, { imgView }, m_SwapChain->extent(), 1 );
            if ( frameBuffer == NULL ) 
                return false;
            m_FrameBuffers.emplace_back( frameBuffer );
        }
        return true;
    }

    bool SimpleRendererVK::createTriangleCommanBuffer()
    {
        m_TriangleCommandBuffer.resize( m_FrameBuffers.size() );
        for ( u32 i=0; i <m_FrameBuffers.size(); i++ )
        {
            m_TriangleCommandBuffer[i] = CommandBufferVK::allocate( m_CommandPool );
            auto b = m_TriangleCommandBuffer[i];
            if ( b == nullptr )
                return false;
            b->beginRecord( CommandBufferUsage::CanResubmit );
            b->beginRenderPass( m_SimplePlass, m_FrameBuffers[i] );
            b->bindPipeline( m_SimplePipeline );
            b->bindVertices( m_TriangleVertexBuffer );
            b->draw( 3, 1, 0, 0 );
            b->endRenderPass();
            b->endRecord();
        }
        return true;
    }

    bool SimpleRendererVK::createFrameSyncObjects()
    {
        for ( u32 i = 0; i < m_SwapChain->numImages(); i++ )
        {
            M<SemaphoreVK> semaphore = SemaphoreVK::create( m_Device );
            if ( semaphore == NULL ) return false;
            m_FrameImageAvailableSemaphore.emplace_back( semaphore );
            // -- triangle
            semaphore = SemaphoreVK::create( m_Device );
            if ( semaphore == NULL ) return false;
            m_FrameTriangleDoneSemaphore.emplace_back( semaphore );
            // submit fence
            M<FenceVK> fence = FenceVK::create( m_Device, true );
            if ( fence == NULL ) return false;
            m_SubmitFences.emplace_back( fence );
        }
        return true;
    }

    bool SimpleRendererVK::createTriangleVertexBuffer()
    {
        void* pMemory;
        u32 queueIndices [] = { m_Device->graphicsQueueFamily() };
        m_TriangleVertexBuffer = BufferVK::create( m_Allocator, sizeof(float)*2*3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, &pMemory, 
                                                   queueIndices, 1 );

        Vec2 positions[3] =
        {
            { 0.0, -0.5 },
            { 0.5, 0.5 },
            { -0.5, 0.5 }
        };

        memcpy( pMemory, positions, sizeof(positions) );

        return m_TriangleVertexBuffer != nullptr;
    }

    bool SimpleRendererVK::createWindow()
    {
        m_Window = OSCreateWindow( "VKWindow", 0, 0, 1600, 900, false );
        return m_Window.invalid() == false;
    }

    void SimpleRendererVK::render()
    {
        u32 imageIdx;
        VkResult imgAcquireResult = m_SwapChain->acquireNextImage( imageIdx, m_FrameImageAvailableSemaphore[m_FrameObjectIdx], NULL );
        if ( imgAcquireResult != VK_SUCCESS )
        {
            return;
        }

        m_SubmitFences[m_FrameObjectIdx]->wait();
        m_SubmitFences[m_FrameObjectIdx]->reset();

        m_GraphicsQueue->submit( m_TriangleCommandBuffer[m_FrameObjectIdx], 
                                 m_FrameImageAvailableSemaphore[m_FrameObjectIdx], 
                                 m_FrameTriangleDoneSemaphore[m_FrameObjectIdx],
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
                                 m_SubmitFences[m_FrameObjectIdx] );

        m_PresentQueue->present( m_SwapChain, imageIdx, m_FrameTriangleDoneSemaphore[m_FrameObjectIdx]  );

        m_FrameObjectIdx = (m_FrameObjectIdx + 1) % m_SwapChain->numImages();
    }


    void SimpleRendererVK::destroyWindow()
    {
        if ( !m_Window.invalid() )
        {
            OSDestroyWindow( m_Window );
            m_Window.setZero();
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL SimpleRendererVK::debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                    void* pUserData )
    {
        switch ( messageSeverity )
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
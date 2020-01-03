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
#include "RenderPassVK.h"
#include "PipelineLayoutVK.h"
#include "PipelineVK.h"
#include "FrameBufferVK.h"
#include "ImageViewVK.h"
#include "HelperVK.h"

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
        if (!createShaders()) return false;
        if (!createSimplePass()) return false;
        if (!createPipelineLayout()) return false;
        if (!createPipeline()) return false;
        if (!createFramebuffer()) return false;

        LOG( "VK Initialized succesful." );
        return true;
    }

    void SimpleRendererVK::closeGraphics()
    {
        destroyWindow();
    }

    void SimpleRendererVK::render()
    {
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

        return m_Device != nullptr;
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
        vset.addAttrib( 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 );
        vset.addAttrib( 0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float)*3 );

        VkViewport vp = HelperVK::makeSimpleViewport( m_SwapChain );

        m_SimplePipeline = PipelineVK::create( m_SimplePlass,
                                               m_EmptyPipelineLayout,
                                               PrimitiveTypeVK::TriangleList,
                                               m_VertexShaderSimple, m_FragmentShaderSimple, NULL, NULL, NULL,
                                               vset, 
                                               vp,
                                               false, false, false,
                                               CullModeVK::Back, PolygonModeVK::Fill, FrontFaceVK::CCW, 
                                               1, 1 );
                                               
       return m_SimplePipeline != nullptr;                                       
    }

    bool SimpleRendererVK::createFramebuffer()
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

    bool SimpleRendererVK::createWindow()
    {
        m_Window = OSCreateWindow( "VKWindow", 0, 0, 1600, 900, false );
        return m_Window.invalid() == false;
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
#include "GraphicResourceVK.h"
#if FV_VULKAN
#include "HelperVK.h"
#include "../Core/LogManager.h"

#define FV_CHECK_GRAPHIC_TYPE(type) \
    if ( m_Type != (type) ) { LOGC("Invalid graphic type for this operation."); } \
    assert( m_Type == (type) );
    

namespace fv
{
    GraphicResourceVK::~GraphicResourceVK()
    {
        freeResource();
    }

    void GraphicResourceVK::init(GraphicType type)
    {
        switch ( type )
        {
        case GraphicType::Texture2D:
        case GraphicType::Texture3D:
        case GraphicType::TextureCube:
            m_Image = nullptr;
            break;
        case GraphicType::Buffer:
            m_Buffer = nullptr;
            break;
        case GraphicType::Shader:
            m_Shader = nullptr;
            break;
        default:
            assert(false);
            break;
        }
    }

    void GraphicResourceVK::freeResource()
    {
        switch ( m_Type )
        {
        case GraphicType::Texture2D:
        case GraphicType::Texture3D:
        case GraphicType::TextureCube:
            if ( m_Device && m_Image ) vkDestroyImage(m_Device, m_Image, nullptr);
            m_Image = nullptr;
            break;
        case GraphicType::Buffer:
            if ( m_Device && m_Buffer ) vkDestroyBuffer(m_Device, m_Buffer, nullptr);
            m_Buffer = nullptr;
            break;
        case GraphicType::Shader:
            if ( m_Device && m_Shader ) vkDestroyShaderModule(m_Device, m_Shader, nullptr);
            m_Shader = nullptr;
            break;
        }
        m_Device = nullptr;
    }

    bool GraphicResourceVK::updateImage(u32 width, u32 height, const byte* data, u32 size, ImageFormat format)
    {
        FV_CHECK_GRAPHIC_TYPE(GraphicType::Texture2D);

        if ((width==0||height==0||data==nullptr||size==0))
        {
            LOGW("Invalid image data, cannot update graphic.");
            return false;
        }
        // TODO vulkan impl
        return true;
    }

    bool GraphicResourceVK::updateBuffer(const byte* data, u32 size, BufferFormat format)
    {
        FV_CHECK_GRAPHIC_TYPE(GraphicType::Buffer);

        return false;
    }

    bool GraphicResourceVK::updateShaderCode(Vector<char>& code)
    {
        FV_CHECK_GRAPHIC_TYPE(GraphicType::Shader);
        return HelperVK::createShaderModule( m_Device, code, m_Shader );
    }

}
#endif
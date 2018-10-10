#include "GraphicResourceVK.h"
#include "../Core/LogManager.h"

#define FV_CHECK_GRAPHIC_TYPE(type) \
    if ( m_Type != (type) ) { LOGC("Invalid graphic type for this operation."); } \
    assert( m_Type == (type) );
    

#if FV_VULKAN
namespace fv
{
    void GraphicResourceVK::freeResource()
    {
        switch ( m_Type )
        {
        case GraphicType::Texture2D:
        case GraphicType::Texture3D:
        case GraphicType::TextureCube:
            vkDestroyImage(m_Device, m_Image, nullptr);
            break;
        case GraphicType::Buffer:
            vkDestroyBuffer(m_Device, m_Buffer, nullptr);
            break;
        case GraphicType::Shader:
            vkDestroyShaderModule(m_Device, m_Shader, nullptr);
            break;
        }
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

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = (const uint32_t*) code.data();
        if ( vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_Shader) != VK_SUCCESS )
        {
            throw std::runtime_error("failed to create shader module!");
        }
        return true;
    }

}
#endif
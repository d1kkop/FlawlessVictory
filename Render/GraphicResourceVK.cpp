#include "GraphicResourceVK.h"
#include "../Core/LogManager.h"

#if FV_VULKAN
namespace fv
{
    FV_TYPE_IMPL(GraphicResourceVK)

    FV_TS bool GraphicResourceVK::updateImage(u32 width, u32 height, const byte* data, u32 size, ImageFormat format)
    {
        if ((width==0||height==0||data==nullptr||size==0))
        {
            LOGW("Invalid image data, cannot update graphic.");
            return false;
        }
        // TODO vulkan impl
        return true;
    }

    FV_TS bool GraphicResourceVK::updateBuffer(const byte* data, u32 size, BufferFormat format)
    {

        return false;
    }

    FV_TS bool GraphicResourceVK::updateShaderCode(Vector<u32>& code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size()*4;
        createInfo.pCode = (const uint32_t*) code.data();
        if ( vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_Shader) != VK_SUCCESS )
        {
            throw std::runtime_error("failed to create shader module!");
        }
        return true;
    }

}
#endif
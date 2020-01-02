#include "ShaderVK.h"
#include "DeviceVK.h"
#include "../../Core/LogManager.h"
#include "../../Core/Common.h"

namespace fv
{
    ShaderVK::~ShaderVK()
    {
        if ( m_Shader )
        {
            vkDestroyShaderModule( m_Device->logical(), m_Shader, NULL );
        }
    }

    M<ShaderVK> ShaderVK::create( const M<DeviceVK>& device, const char* code, u32 size )
    {
        VkShaderModule shaderModule;
        VkShaderModuleCreateInfo createInfo ={};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = size;
        createInfo.pCode = (const uint32_t*)code;
        if ( vkCreateShaderModule( device->logical(), &createInfo, nullptr, &shaderModule ) != VK_SUCCESS )
        {
            LOGC( "Failed to create shader module." );
            return {};
        }
        M<ShaderVK> shaderVk = std::make_shared<ShaderVK>();
        shaderVk->m_Shader = shaderModule;
        shaderVk->m_Device = device;
        return shaderVk;
    }

    M<ShaderVK> ShaderVK::create( const M<DeviceVK>& device, const List<char>& data )
    {
        return create( device, data.data(), (u32) data.size() );
    }

}
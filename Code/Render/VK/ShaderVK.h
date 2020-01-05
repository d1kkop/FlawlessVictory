#pragma once
#include "IncVulkan.h"
#include "../DeviceResource.h"
#include "../../Core/Common.h"

namespace fv
{
    class DeviceVK;

    class ShaderVK : public DeviceResource
    {
    public:
        ShaderVK() = default;
        ~ShaderVK() override;

    public:
        static M<ShaderVK> create( const M<DeviceVK>& device, const char* data, u32 size );
        static M<ShaderVK> create( const M<DeviceVK>& device, const List<char>& data );

        VkShaderModule vk() const { return m_Shader; }

    private:
        VkShaderModule m_Shader {};
        M<DeviceVK> m_Device;
    };
}
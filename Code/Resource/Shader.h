#pragma once
#include "Resource.h"
#include "../Render/RenderManager.h"

namespace fv
{
    class DeviceResource;

    class Shader: public Resource
    {
        FV_TYPE(Shader)

    public:
        FV_DLL ~Shader() override;
        FV_MO void setDeviceShader( const M<DeviceResource>& deviceShader );

    private:
        FV_DLL void load_RT(const ResourceToLoad& rtl) override;

        M<DeviceResource> m_DeviceShader;
    };

    using Shdr = M<Shader>;
}
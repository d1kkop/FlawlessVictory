#pragma once
#include "Resource.h"
#include "../Render/RenderManager.h"

namespace fv
{
    class Shader: public Resource
    {
        FV_TYPE(Shader)

    public:
        FV_DLL ~Shader() override;
        FV_MO void applyPatch( const RShader& graphic );

    private:
        FV_DLL void load(const ResourceToLoad& rtl) override;

        RShader m_Graphic{};
    };
}
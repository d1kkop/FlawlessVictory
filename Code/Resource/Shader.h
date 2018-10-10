#pragma once
#include "Resource.h"

namespace fv
{
    class GraphicResource;

    class Shader: public Resource
    {
        FV_TYPE(Shader)

    public:
        FV_DLL ~Shader() override;
        FV_MO void applyPatch( GraphicResource* graphic );

    private:
        FV_DLL void load(const ResourceToLoad& rtl) override;

        GraphicResource* m_Graphic{};
    };
}
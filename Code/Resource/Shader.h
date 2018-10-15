#pragma once
#include "Resource.h"

namespace fv
{
    class Shader: public Resource
    {
        FV_TYPE(Shader)

    public:
        FV_DLL ~Shader() override;
        FV_MO void applyPatch( u64 graphic );

    private:
        FV_DLL void load(const ResourceToLoad& rtl) override;

        u64 m_Graphic = -1;
    };
}
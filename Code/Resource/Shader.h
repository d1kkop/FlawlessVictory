#pragma once
#include "Resource.h"

namespace fv
{
    class Shader: public Resource
    {
        FV_RESOURCE_TYPE(Shader)

    public:
        FV_DLL ~Shader() override;

    private:
        FV_DLL void load(const ResourceToLoad& rtl) override;

        class GraphicResource* m_Graphic;

        friend class ResourceManager;
    };
}
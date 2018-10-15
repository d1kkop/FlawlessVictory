#pragma once
#include "Resource.h"
#include "../Render/RenderManager.h"

namespace fv
{
    class Material: public Resource
    {
        FV_TYPE(Material)

    public:
        MaterialData data;
    };
}
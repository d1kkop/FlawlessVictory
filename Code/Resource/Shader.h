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
        FV_DLL void load(const Path& path) override;
        FV_DLL void onDoneOrCancelled(class Job* j) override;

        class GraphicResource* m_Graphic;

        friend class ResourceManager;
    };
}
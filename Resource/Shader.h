#pragma once
#include "Resource.h"

namespace fv
{
    class Shader: public Resource
    {
        FV_RESOURCE_TYPE(Shader, "")

    public:
        FV_DLL ~Shader() override;
        FV_DLL void load(const Path& path) override;
        FV_DLL void onDoneOrCancelled(class Job* j) override;

    private:
        // Never return ptr to resource directly. Always work from managed resource.
        class GraphicResource* m_Graphic {};
    };
}
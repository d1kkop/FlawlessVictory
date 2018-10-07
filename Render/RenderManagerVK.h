#pragma once
#include "RenderManager.h"

#if FV_VULKAN
namespace fv
{
    class GraphicResource;

    class RenderManagerVK : public RenderManager
    {
    public:
        FV_DLL GraphicResource* createGraphic() override;
    };
}
#endif
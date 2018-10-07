#include "RenderManagerVK.h"
#include "GraphicsResourceVK.h"
#include "../Core/ComponentManager.h"

#if FV_VULKAN
namespace fv
{
    GraphicResource* RenderManagerVK::createGraphic()
    {
        // Recycle GraphicResources.
        GraphicResource* graphic = sc<GraphicResource*>(componentManager()->newComponent(GraphicResourceVK::type()));
        return graphic;
    }
}
#endif
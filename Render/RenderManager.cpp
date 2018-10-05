#include "RenderManager.h"
#include "GraphicResource.h"
#include "../Core/Functions.h"
#include "../Core/ComponentManager.h"

namespace fv
{
    void RenderManager::render(const class Camera* camera)
    {

    }

    GraphicResource* RenderManager::createGraphic(u32 type)
    {
        // Recycle GraphicResources.
        return sc<GraphicResource*>( componentManager()->newComponent(type) );
    }

    void RenderManager::freeGraphic(GraphicResource* graphic)
    {
        if (!graphic) return;
        componentManager()->freeComponent( graphic );
    }

    RenderManager* g_RenderManager {};
    RenderManager* renderManager() { return CreateOnce(g_RenderManager); }
    void deleteRenderManager() { delete g_RenderManager; g_RenderManager=nullptr; }
}
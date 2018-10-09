#include "RenderManagerVK.h"
#include "GraphicResourceVK.h"
#include "../Core/Functions.h"
#include "../Core/ComponentManager.h"

namespace fv
{
    void RenderManager::setResourceType(GraphicResource* gr, u32 resourceType)
    {
        assert( gr );
        gr->m_ResourceType = resourceType;
    }

    RenderManager* g_RenderManager {};
    RenderManager* renderManager() 
    { 
    #if FV_VULKAN
        auto rvk = (RenderManagerVK*)g_RenderManager;
        g_RenderManager = CreateOnce<RenderManagerVK>( rvk );
    #else
    #pragma error ("No implementation")
    #endif
        return g_RenderManager;
    }
    void deleteRenderManager() { delete g_RenderManager; g_RenderManager=nullptr; }
}
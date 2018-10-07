#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"

namespace fv
{
    class GraphicResource;

    class RenderManager
    {
    public:
        virtual void render(const class Camera* camera);
        FV_DLL virtual GraphicResource* createGraphic() = 0;
        FV_DLL void freeGraphic(GraphicResource* graphic);

        template <class T> GraphicResource* createGraphic();
    };


    template <class T> 
    GraphicResource* RenderManager::createGraphic()
    {
        GraphicResource* graphic = createGraphic();
        graphic->m_ResourceType = T::type();
        return graphic;
    }

    FV_DLL RenderManager* renderManager();
    FV_DLL void deleteRenderManager();
}
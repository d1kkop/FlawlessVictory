#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"

namespace fv
{
    class GraphicResource;

    struct RenderManagerParams
    {
        Vector<void*> windowHandles;
    };

    class RenderManager
    {
    public:
        virtual ~RenderManager() = default;
        virtual bool initGraphics(const RenderManagerParams& params) = 0;
        virtual void closeGraphics() = 0;
        virtual GraphicResource* createGraphic() = 0;
        virtual void render(const class Camera* camera) = 0;

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
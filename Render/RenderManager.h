#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"

namespace fv
{
    class GraphicResource;

    class RenderManager
    {
    public:
        void render(const class Camera* camera);
        FV_DLL GraphicResource* createGraphic(u32 type);

        template <class T> GraphicResource* createGraphic();
    };


    template <class T> 
    GraphicResource* RenderManager::createGraphic()
    {
        return createGraphic(T::type());
    }

    FV_DLL RenderManager* renderManager();
    FV_DLL void deleteRenderManager();
}
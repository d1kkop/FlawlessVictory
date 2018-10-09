#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"

namespace fv
{
    class GraphicResource;

    struct RenderSetup
    {
        bool createMainWindow;
        bool creatSecondaryWindow;
        String mainWindowName;
        String secondaryWindowName;
        u32 mainWindowWidth, mainWindowHeight;
        u32 secondaryWindowWidth, secondaryWindowHeight;
        bool mainWindowFullscreen;
        bool secondaryWindowFullscreen;
    };

    class RenderManager
    {
    public:
        virtual ~RenderManager() = default;
        virtual bool initGraphics() = 0;
        virtual void closeGraphics() = 0;
        FV_TS virtual GraphicResource* createGraphic(u32 resourceType, u32 deviceIdx=0) = 0;
        FV_TS virtual void freeGraphic(GraphicResource* graphic) = 0;

        template <class T> 
        FV_TS GraphicResource* createGraphic(u32 deviceIdx=0);

        static void setResourceType( GraphicResource* gr, u32 resourceType );
    };


    template <class T> 
    GraphicResource* RenderManager::createGraphic(u32 deviceIdx)
    {
        return createGraphic(T::type());
    }

    FV_DLL RenderManager* renderManager();
    FV_DLL void deleteRenderManager();
}
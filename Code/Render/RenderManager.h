#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"

namespace fv
{
    class GraphicResource;
    enum class GraphicType;

    struct RenderConfig
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
        FV_TS virtual GraphicResource* createGraphic(GraphicType type, u32 deviceIdx=0) = 0;
        FV_TS virtual void freeGraphic(GraphicResource* graphic, bool async=false) = 0;

        static void setGraphicType( GraphicResource* gr, GraphicType type );
    };


    FV_DLL RenderManager* renderManager();
    FV_DLL void deleteRenderManager();
}
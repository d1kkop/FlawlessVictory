#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"

namespace fv
{
    class GraphicResource;
    enum class GraphicType;

    struct RenderConfig
    {
        u32 resX, resY;
        u32 numSamples;     // Msaa, default 1
        u32 numImages;      // Num imgages to render to, 3 for tripple buffering, 2 for double
        u32 numLayers;      // Num layers 1, or 2 for stereo 3d
        bool createWindow;
        u32 windowWidth, windowHeight;
        bool fullScreen;
        String windowName;
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

        // TEMP
        virtual void drawTriangle( const Vec3& v1, const Vec3& v2, const Vec3& v3 ) = 0;
    };


    FV_DLL RenderManager* renderManager();
    FV_DLL void deleteRenderManager();
}
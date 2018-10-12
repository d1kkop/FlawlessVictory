#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"

namespace fv
{
    class GraphicResource;
    enum class GraphicType;

    struct RenderConfig
    {
        u32 maxDevices = (u32)-1;
        u32 resX, resY;
        u32 numSamples;     // Msaa, default 1
        u32 numImages;      // Num imgages to render to, 3 for tripple buffering, 2 for double
        u32 numLayers;      // Num layers 1, or 2 for stereo 3d
        u32 numFramesBehind; // Max allowed frames in flight before sync on host
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
        void readRenderConfig();
        virtual void drawFrame() = 0;
        virtual void waitOnDeviceIdle() = 0;

    protected:
        RenderConfig m_RenderConfig{};
    };


    FV_DLL RenderManager* renderManager();
    FV_DLL void deleteRenderManager();
}
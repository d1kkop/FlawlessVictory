#include "RenderManagerVK.h"
#include "../Core/Functions.h"
#include "../Core/ComponentManager.h"

namespace fv
{
    void RenderManager::readRenderConfig()
    {
        auto& rs = m_RenderConfig;
        // TODO read from config
        rs.maxDevices = 1; // TODO should work with more and with a single swap chain
        rs.resX = 1200;
        rs.resY = 1000;
        rs.numImages = 3;
        rs.numSamples = 1; // For msaa 2, 4 etc
        rs.numLayers = 1; // for stereo 2
        rs.numFramesBehind = 2;
        rs.createWindow = true; // TODO without swap chain, the draw frame hangs on wait for Fence
        rs.windowWidth = rs.resX;
        rs.windowHeight = rs.resY;
        rs.windowName = "Main Window";
        rs.fullScreen = false;
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
#include "RenderManagerVK.h"
#include "../Core/Functions.h"
#include "../Core/ComponentManager.h"

namespace fv
{
    // -------------------- SubmeshInput -----------------------------------------------------------------------------------------------

    u32 SubmeshInput::computeNumComponents() const
    {
        u32 vertexComponentCount = 0;
        if ( positions )  vertexComponentCount += 3;
        if ( normals )    vertexComponentCount += 3;
        if ( tanBins )    vertexComponentCount += 6; // tan and bitangents
        if ( uvs )        vertexComponentCount += 2;
        if ( lightUvs )   vertexComponentCount += 2;
        if ( bones )      vertexComponentCount += 5; // weights and boneIndices
        for ( u32 i=0; i<4; ++i )
        {
            if ( extras[i] ) vertexComponentCount += 4;
        }
        return vertexComponentCount;
    }

    // -------------------- Submesh -----------------------------------------------------------------------------------------------

    u32 Submesh::getVertexCount() const
    {
        if ( vertices.size() ) return (u32) vertices.size();
        if ( normals.size() ) return (u32) normals.size();
        if ( tangents.size() && bitangents.size() ) return (u32) tangents.size();
        if ( uvs.size() ) return (u32) uvs.size();
        if ( lightUVs.size() ) return (u32) lightUVs.size();
        if ( weights.size() && boneIndices.size() ) return (u32) boneIndices.size();
        if ( extra1.size() ) return (u32) extra1.size();
        if ( extra2.size() ) return (u32) extra2.size();
        if ( extra3.size() ) return (u32) extra3.size();
        if ( extra4.size() ) return (u32) extra4.size();
        return 0;
    }

    // -------------------- RenderManager -----------------------------------------------------------------------------------------------

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
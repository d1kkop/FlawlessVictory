#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"
#include "../Core/OSLayer.h"
#include "DeviceResource.h"

namespace fv
{
    enum class GraphicType;

    struct RenderConfig
    {
        u32 maxDevices = (u32)-1;
        u32 resX, resY;
        u32 numSamples;     // Msaa, default 1
        u32 numImages;      // Num imgages to render to, 3 for tripple buffering, 2 for double
        u32 numLayers;      // Num layers 1, or 2 for stereo 3d
        u32 numFramesBehind; // Max allowed frames in flight before sync on host
        u32 numCommandsBeforeSubmit = 250;
        bool createWindow;
        u32 windowWidth, windowHeight;
        bool fullScreen;
        String windowName;
    };

    // Using this instead of many small heap allocations that are slow to access (cache miss) compared to stack memory.

    using RSubmesh = void*;
    using RTexture2D = void*;
    using RMaterial = void*;
    using RRenderPass = void*;

    enum class BufferFormat
    {
        Default,
        Optimized
    };

    enum class DrawMethod
    {
        FillStandard,
        FillTransparent,
        WireFrame,
        Count
    };

    enum class ImageFormat
    {
        RGBA8,
        RGB8,
        Single8,
        RGBA16,
        RGB16,
        RGBA32F,
        RGB32F,
        RGB555
    };

    enum VertexType
    {
        Position,
        Normal,
        Tangent,
        Bitangent,
        Uv,
        LightUv,
        Extra1,
        Extra2,
        Extra3,
        Extra4,
        Weights,
        BoneIndices,
        COUNT
    };

    struct Submesh
    {
        Vector<u32>  indices;
        Vector<Vec3> vertices;
        Vector<Vec3> normals;
        Vector<Vec3> tangents;
        Vector<Vec3> bitangents;
        Vector<Vec2> uvs;
        Vector<Vec2> lightUVs;
        Vector<Vec4> weights;
        Vector<u32>  boneIndices;
        Vector<Vec4> extra1;
        Vector<Vec4> extra2;
        Vector<Vec4> extra3;
        Vector<Vec4> extra4;
        Vec3 bMin {};
        Vec3 bMax {};

        // As vertices are not mandatory, vertices.size() may return 0.
        u32 getVertexCount() const;
    };

    struct SubmeshInput
    {
        bool positions;
        bool normals;
        bool tanBins;
        bool uvs;
        bool lightUvs;
        bool extras[4];
        bool bones;
        u32 computeNumComponents() const;
    };


    class RenderManager
    {
    public:
        virtual ~RenderManager() = default;
        virtual bool initGraphics() = 0;
        virtual void closeGraphics() = 0;
        virtual void render() = 0;

        virtual M<DeviceResource> createShader( const char* code, u32 size ) = 0;

        const OSHandle& getWindowHandle() const { return m_Window; }

    protected:
        OSHandle m_Window {};
    };


    FV_DLL RenderManager* renderManager();
    FV_DLL void deleteRenderManager();
}
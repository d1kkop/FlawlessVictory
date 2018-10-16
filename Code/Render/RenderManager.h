#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"

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
        bool createWindow;
        u32 windowWidth, windowHeight;
        bool fullScreen;
        String windowName;
    };

    using HShader = void*;
    using HTexture2D = void*;
    using HMaterial = void*;
    using HRenderPass = void*;

    enum class BufferFormat
    {
        Default,
        Optimized
    };

    enum class ImageFormat
    {
        RGBA8,
        RGB8,
        Single8,
        RGBA16,
        RGBA32F,
        RGB16,
        RGB32F,
        RGB555,
        RGB565
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
        Vector<byte> boneIndices;
        Vector<Vec4> extra1;
        Vector<Vec4> extra2;
        Vector<Vec4> extra3;
        Vector<Vec4> extra4;
        Vec3 bMin {};
        Vec3 bMax {};
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
    };

    struct MaterialData
    {
        HShader vertShader;
        HShader fragShader;
        HShader geomShader;
    };


    class RenderManager
    {
    public:
        virtual ~RenderManager() = default;
        virtual bool initGraphics() = 0;
        virtual void closeGraphics() = 0;

        virtual u64 createTexture2D( u32 width, u32 height, const char* data, u32 size, ImageFormat format ) = 0;
        virtual u64 createShader( const char* data, u32 size ) = 0;
        virtual u64 createSubmesh( const Submesh& submesh, const MaterialData& matData ) = 0;
        virtual void deleteTexture2D( u64 tex2d ) = 0;
        virtual void deleteShader( u64 shader ) = 0;
        virtual void deleteSubmesh( u64 submesh ) = 0;

        void readRenderConfig();
        virtual void drawFrame() = 0;
        virtual void waitOnDeviceIdle() = 0;

    protected:
        RenderConfig m_RenderConfig{};
    };


    FV_DLL RenderManager* renderManager();
    FV_DLL void deleteRenderManager();
}
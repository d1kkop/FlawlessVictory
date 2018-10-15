#pragma once
#include "../Core/Common.h"
#include "../Core/Object.h"


namespace fv
{
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

    enum class GraphicType
    {
        Texture2D,
        Texture3D,
        TextureCube,
        Buffer,
        Shader,
        Submesh
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
        bool normals;
        bool tanBins;
        bool uvs;
        bool lightUvs;
        bool extras[4];
        bool bones;
    };

    struct MaterialData
    {
        Map<String, Vec4> uniforms;
    };

    class GraphicResource: public Object
    {
    public:
        virtual ~GraphicResource() = default;
        virtual void init(GraphicType type) = 0;
        virtual bool updateImage(u32 width, u32 height, const byte* data, u32 size, ImageFormat format) = 0;
        virtual bool updateBuffer(const byte* data, u32 size, BufferFormat format) = 0;
        virtual bool updateShaderCode(Vector<char>& code) = 0;
        virtual bool updateMeshData(const struct Submesh& submesh, const SubmeshInput& sinput, const MaterialData& matData) = 0;

    protected:
        GraphicType m_Type;

        friend class RenderManager;
    };

}
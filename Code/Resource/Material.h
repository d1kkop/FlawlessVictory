#pragma once
#include "Resource.h"
#include "../Render/RenderManager.h"

namespace fv
{
    class Texture2D;
    class Shader;

    enum struct Binding
    {
        Diffuse,
        Ambient,
        Specular,
        Metallic,
        Roughness,
        Normal,
        Emissive
    };

    enum struct SamplerFilter
    {
        Point,
        Bilinear,
        Trilinear,
        An1x,
        An2x,
        An4x,
        An8x,
        An16x
    };

    enum struct SamplerClampMode
    {
        Clamp,
        Repeat
    };

    struct Sampler2D
    {
        u32 binding;
        u32 location;
        M<Texture2D> texture;
        SamplerFilter filter;
        SamplerClampMode clamp;
        u32 minLod;
        u32 maxLod;
        u32 baseLod;
    };

    struct Uniform
    {
        u32 binding;
        u32 location;
        Vec4 data;
        u32 numComponents;
    };


    class Material: public Resource
    {
        FV_TYPE(Material)

    public:
        Vector<Sampler2D> m_Samplers;
        Vector<Uniform> m_Uniforms;
        M<Shader> m_VertShader;
        M<Shader> m_FragShader;
        M<Shader> m_GeomShader;
    };

    using Mat = M<Material>;
}
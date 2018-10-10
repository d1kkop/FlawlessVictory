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
        Shader
    };

    class GraphicResource: public Object
    {
    public:
        virtual ~GraphicResource() = default;
        virtual void init(GraphicType type) = 0;
        virtual void freeResource() = 0; // TODO Make private somehow
        virtual bool updateImage(u32 width, u32 height, const byte* data, u32 size, ImageFormat format) = 0;
        virtual bool updateBuffer(const byte* data, u32 size, BufferFormat format) = 0;
        virtual bool updateShaderCode(Vector<char>& code) = 0;

    protected:
        GraphicType m_Type;

        friend class RenderManager;
    };

}
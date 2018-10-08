#pragma once
#include "../Core/Common.h"
#include "../Core/Component.h"


namespace fv
{
    enum BufferFormat
    {
        Default,
        Optimized
    };

    enum ImageFormat
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

    class GraphicResource: public Component
    {
    public:
        FV_TS virtual bool updateImage( u32 width, u32 height, const byte* data, u32 size, ImageFormat format ) = 0;
        FV_TS virtual bool updateBuffer( const byte* data, u32 size, BufferFormat format ) = 0;
        FV_TS virtual bool updateShaderCode( Vector<u32>& code ) = 0;

    protected:
        u32 m_ResourceType;

        friend class RenderManager;
    };
}
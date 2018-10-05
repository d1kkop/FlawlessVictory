#pragma once
#include "GraphicResource.h"

#if FV_VULKAN
namespace fv
{
    class GraphicResourceVK: public GraphicResource
    {
        FV_TYPE(GraphicResourceVK, 0, false)

        FV_TS bool updateImage( u32 width, u32 height, const byte* data, u32 size, ImageFormat format ) override;
        FV_TS bool updateBuffer( const byte* data, u32 size, BufferFormat format ) override;
    };
}
#endif
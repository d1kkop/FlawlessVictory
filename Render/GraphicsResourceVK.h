#pragma once
#include "GraphicResource.h"

#if FV_VK
namespace fv
{
    class GraphicResourceVK: public GraphicResource
    {
        FV_TS void updateBuffer( const byte* data, u32 size, u32 format ) override;
    };
}
#endif
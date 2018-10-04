#pragma once
#include "../Core/Common.h"

namespace fv
{
    class GraphicResource
    {
    public:
        FV_TS virtual void updateBuffer( const byte* data, u32 size, u32 format ) = 0;
    };
}
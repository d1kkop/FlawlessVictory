#pragma once
#include "../Core/Common.h"

namespace fv
{
    class Resource;

    class ResourceManager
    {
    public:
        FV_TS Resource* load(u32 type);

        template <class T> FV_TS T* load();
    };
}

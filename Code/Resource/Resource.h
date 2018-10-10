#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"

namespace fv
{
    struct ResourceToLoad;

    class Resource: public Type
    {
    public:
        virtual ~Resource() = default;

        // Called from different reseource load thread.
        virtual void load(const ResourceToLoad& path) { };
    };
}

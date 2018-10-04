#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"

namespace fv
{
    class Resource: public Type
    {
    public:
        // Called from different thread.
        virtual void load(const Path& path) { };
        virtual void onDoneOrCancelled(class Job* loadJob) { };
    };
}

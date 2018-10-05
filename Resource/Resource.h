#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"

namespace fv
{
    class Resource: public Type
    {
    public:
        virtual ~Resource() = default;

        // Called from different thread.
        virtual void load(const Path& path) { };
        virtual void onDoneOrCancelled(class Job* loadJob) { };

        bool loadDone() const { return m_LoadDone; }

    protected:
        Atomic<bool> m_LoadDone;
        Atomic<bool> m_LoadSuccesful;
    };
}

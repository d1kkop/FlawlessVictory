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

        // Called from resources load thread.
        virtual void load_RT(const ResourceToLoad& path) { };

        const String& filename() const { return m_Filename; }

    private:
        String m_Filename;

        friend class ResourceManager;
    };

    using Res = M<Resource>;
}

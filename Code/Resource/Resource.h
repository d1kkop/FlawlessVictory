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

        FV_DLL const String& filename() const { return m_Filename; }

    private:
        String m_Filename;

        friend class ResourceManager;
    };
}

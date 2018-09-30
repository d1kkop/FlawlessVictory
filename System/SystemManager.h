#pragma once
#include "../Core/Common.h"

namespace fv
{
    struct SystemParams
    {
        String moduleName;
    };

    class SystemManager
    {
    public:
        bool initialize( const SystemParams& params );
        void mainloop();
        const SystemParams& startParams() const { return m_Params; }

    private:
        bool m_Done = false;
        SystemParams m_Params;
    };

    class SystemManager* systemManager();
}
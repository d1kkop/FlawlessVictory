#pragma once
#include "../Core/Common.h"
#include "../Core/Thread.h"

namespace fv
{
    struct SystemParams
    {
        String moduleName;
    };

    class SystemManager
    {
    public:
        FV_DLL bool initialize( const SystemParams& params );
        FV_DLL void mainloop();
        const SystemParams& startParams() const { return m_Params; }

    private:
        bool m_Done = false;
        SystemParams m_Params;
        Vector<Thread> m_Threads;
    };

    FV_DLL class SystemManager* systemManager();
    FV_DLL void deleteSystemManager();
}
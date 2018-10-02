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
        bool initialize( const SystemParams& params );
        void mainloop();
        const SystemParams& startParams() const { return m_Params; }

    private:
        bool m_Done = false;
        SystemParams m_Params;
        Vector<Thread> m_Threads;
    };

    class SystemManager* systemManager();
    void deleteSystemManager();
}
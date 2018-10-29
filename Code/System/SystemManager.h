#pragma once
#include "../Core/Common.h"
#include "../Core/Thread.h"
#include "../Core/ComponentManager.h"

namespace fv
{
    struct SystemParams
    {
        i32 argc;
        char** argv;
        String moduleName;
    };

    class SystemManager
    {
    public:
        FV_DLL bool initialize( const SystemParams& params );
        FV_DLL void mainloop();
        const SystemParams& startParams() const { return m_Params; }

    private:
        void prepareSortListFor( Map<u32, Vector<ComponentArray>>& components, Vector<ComponentArray>& sortedList );

        bool m_Done = false;
        SystemParams m_Params {};
        Vector<ComponentArray> m_SortedUpdatables;
        Vector<ComponentArray> m_SortedOthers;
    };

    FV_DLL class SystemManager* systemManager();
    FV_DLL void deleteSystemManager();
}
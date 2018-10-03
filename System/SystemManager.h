#pragma once
#include "../Core/Common.h"
#include "../Core/Thread.h"

namespace fv
{
    struct SystemParams
    {
        String moduleName;
        u32 windowWidth;
        u32 windowHeight;
        bool fullscreen;
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
        void* m_Window {};
    };

    FV_DLL class SystemManager* systemManager();
    FV_DLL void deleteSystemManager();
}
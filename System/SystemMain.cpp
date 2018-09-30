#include "SystemMain.h"
#include "SystemManager.h"
#include "../Core.h"
#include "../Scene.h"
#include "../Render.h"

namespace fv
{
    SystemManager* g_SystemManager {};

    FV_DLL void system_start()
    {
        core_start();
        scene_start();
        render_start();
    }

    FV_DLL void system_close()
    {
        render_close();
        scene_close();
        core_close();
        deleteAndNull( g_SystemManager );
    }

    class SystemManager* systemManager() { return createOnce(g_SystemManager); }

}
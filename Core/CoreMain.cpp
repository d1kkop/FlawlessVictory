#include "CoreMain.h"
#include "Functions.h"
#include "TypeManager.h"
#include "ComponentManager.h"
#include "LogManager.h"

namespace fv
{
    TypeManager* g_TypeManager {};
    ComponentManager* g_ComponentManager {};
    LogManager* g_LogManager {};

    FV_DLL void core_start()
    {
        if ( g_TypeManager ) return; // Assert only once started
        g_LogManager = new LogManager();
        g_TypeManager = new TypeManager();
        g_ComponentManager = new ComponentManager();
    }

    FV_DLL void core_close()
    {
        deleteAndNull(g_ComponentManager);
        deleteAndNull(g_TypeManager);
        deleteAndNull(g_LogManager);
    }

    TypeManager* typeManager() { return g_TypeManager; }
    ComponentManager* componentManager() { return g_ComponentManager; }
    LogManager* logManager() { return g_LogManager; }
}
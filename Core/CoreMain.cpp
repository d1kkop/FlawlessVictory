#include "CoreMain.h"
#include "Functions.h"
#include "TypeManager.h"
#include "ComponentManager.h"
#include "LogManager.h"

namespace fv
{
    bool g_ExecutingParallel = false;

    TypeManager* g_TypeManager {};
    ComponentManager* g_ComponentManager {};
    LogManager* g_LogManager {};

    FV_DLL void core_start()
    {
    }

    FV_DLL void core_close()
    {
        deleteAndNull(g_ComponentManager);
        deleteAndNull(g_TypeManager);
        deleteAndNull(g_LogManager);
    }

    FV_DLL bool isExecutingParallel() { return g_ExecutingParallel; }
    FV_DLL void setExecutingParallel(bool isParallel) { g_ExecutingParallel=isParallel; }

    // Type and LogManager are already needed at static initialization.
    TypeManager* typeManager() { return createOnce(g_TypeManager); }
    LogManager* logManager() { return createOnce(g_LogManager); }
    ComponentManager* componentManager() { return createOnce(g_ComponentManager); }
}
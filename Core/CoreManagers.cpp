#include "CoreManagers.h"
#include "TypeManager.h"
#include "ComponentManager.h"

namespace fv
{
    TypeManager* g_TypeManager {};
    ComponentManager* g_ComponentManager {};

    void core_start()
    {
        g_TypeManager = new TypeManager();
        g_ComponentManager = new ComponentManager();
    }

    void core_close()
    {
        deleteAndNull(g_ComponentManager);
        deleteAndNull(g_TypeManager);
    }

    TypeManager* typeManager() { return g_TypeManager; }
    ComponentManager* componentManager() { return g_ComponentManager; }
}
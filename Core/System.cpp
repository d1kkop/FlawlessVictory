#include "System.h"
#include "ObjectManager.h"
#include "ComponentManager.h"


namespace fv
{
    /* Managers */
    ObjectManager* g_ObjectManager;
    ComponentManager* g_ComponentManager;
    TypeManager* g_TypeManager;


    void startup()
    {
         g_ObjectManager     = new ObjectManager{};
         g_ComponentManager  = new ComponentManager{};
         g_TypeManager       = new TypeManager{};
    }

    void shutdown()
    {
        deleteAndNull( g_TypeManager );
        deleteAndNull( g_ComponentManager );
        deleteAndNull( g_ObjectManager );
    }


    ObjectManager* objectManager() { return g_ObjectManager; }
    ComponentManager* componentManager() { return g_ComponentManager; }
    TypeManager* typeManager() { return g_TypeManager; }
}
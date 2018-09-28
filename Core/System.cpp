#include "System.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "ComponentManager.h"


namespace fv
{
    /* Managers */
    ObjectManager<GameObject>* g_GameObjectManager;
    ComponentManager* g_ComponentManager;
    TypeManager* g_TypeManager;


    void startup()
    {
         g_GameObjectManager     = new ObjectManager<GameObject>{};
         g_ComponentManager  = new ComponentManager{};
         g_TypeManager       = new TypeManager{};
    }

    void shutdown()
    {
        deleteAndNull( g_TypeManager );
        deleteAndNull( g_ComponentManager );
        deleteAndNull( g_GameObjectManager );
    }


    ObjectManager<GameObject>* gameObjectManager() { return g_GameObjectManager; }
    ComponentManager* componentManager() { return g_ComponentManager; }
    TypeManager* typeManager() { return g_TypeManager; }
}
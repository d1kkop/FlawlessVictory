#include "SceneManagers.h"

namespace fv
{
    ObjectManager<GameObject>* g_GameObjectManager {};
    ObjectManager<SceneWorld>* g_SceneWorldManager {};

    FV_DLL void scene_start()
    {
        g_GameObjectManager = new ObjectManager<GameObject>();
        g_SceneWorldManager = new ObjectManager<SceneWorld>();
    }

    FV_DLL void scene_close()
    {
        deleteAndNull( g_GameObjectManager );
        deleteAndNull( g_SceneWorldManager );
    }

    ObjectManager<SceneWorld>* sceneWorldManager() { return g_SceneWorldManager; }
    ObjectManager<GameObject>* gameObjectManager() { return g_GameObjectManager; }

}

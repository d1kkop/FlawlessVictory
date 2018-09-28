#include "SceneManagers.h"

namespace fv
{
    ObjectManager<GameObject>* g_GameObjectManager {};

    FV_DLL void scene_start()
    {
        g_GameObjectManager = new ObjectManager<GameObject>();
    }

    FV_DLL void scene_close()
    {
        deleteAndNull( g_GameObjectManager );
    }


    ObjectManager<fv::GameObject>* gameObjectManager() { return g_GameObjectManager; }
}

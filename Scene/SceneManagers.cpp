#include "SceneManagers.h"

namespace fv
{
    ObjectManager<GameObject>* g_GameObjectManager {};

    void scene_start()
    {
        g_GameObjectManager = new ObjectManager<GameObject>();
    }

    void scene_close()
    {
        deleteAndNull( g_GameObjectManager );
    }
}

#include "SceneMain.h"
#include "../Core/Functions.h"

namespace fv
{
    ObjectManager<GameObject>* g_GameObjectManager {};

    FV_DLL void scene_start()
    {
    }

    FV_DLL void scene_close()
    {
        deleteAndNull( g_GameObjectManager );
    }

    ObjectManager<GameObject>* gameObjectManager() { return createOnce(g_GameObjectManager); }

}

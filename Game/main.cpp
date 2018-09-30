#include "../Core.h"
#include "../Scene.h"
#include "../Render.h"
using namespace fv;


int main(int argc, char** arv)
{
    fv::core_start();
    fv::scene_start();

    GameObject* o = gameObjectManager()->newObject();
    o->addComponent<SceneComponent>();
    
    GameObject* o2 = gameObjectManager()->newObject();
    o->addComponent<SceneComponent>();

    GameObject* cam = gameObjectManager()->newObject();
    cam->addComponent<Camera>();

    float s = time();
    
    fv::scene_close();
    fv::core_close();
    return 0;
}
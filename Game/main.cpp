#include "../Core.h"
#include "../Scene.h"
using namespace fv;


int main(int argc, char** arv)
{
    fv::core_start();
    fv::scene_start();

    SceneWorld* scene = sceneWorldManager()->newObject();
    GameObject* o = gameObjectManager()->newObject();

    o->addComponent<SceneComponent>(scene);
    bool bHas = o->hasComponent<SceneComponent>();
    bHas = o->hasComponent<SceneComponent>();
    o->removeComponent<SceneComponent>();

    printf("Num %d\n", o->numComponents() );
    o->removeComponent<SceneComponent>();
    printf("Num %d\n", o->numComponents() );
    
    fv::scene_close();
    fv::core_close();
    return 0;
}
#include "../Core.h"
#include "../Scene.h"
using namespace fv;


int main(int argc, char** arv)
{
    fv::startup();

    GameObject* o = gameObjectManager()->newObject();
    o->addOrGetComponent<SceneComponent>();
    bool bHas = o->hasComponent<SceneComponent>();
    bHas = o->hasComponent<SceneComponent>();
    o->removeComponent<SceneComponent>();

    
    printf("Num %d\n", o->numComponents() );
    o->removeComponent<SceneComponent>();
    printf("Num %d\n", o->numComponents() );
    
    fv::shutdown();
    return 0;
}
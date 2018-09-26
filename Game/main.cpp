#include "../Core.h"
using namespace fv;


int main(int argc, char** arv)
{
    fv::startup();

    Object* e = objectManager()->newObject();
    e->addOrGetComponent<SceneComponent>();
    bool bHas = e->hasComponent<SceneComponent>();
    bHas = e->hasComponent<SceneComponent>();
    e->removeComponent<SceneComponent>();

    
    printf("Num %d\n", e->numComponents() );
    e->removeComponent<SceneComponent>();
    printf("Num %d\n", e->numComponents() );
    
    fv::shutdown();
    return 0;
}
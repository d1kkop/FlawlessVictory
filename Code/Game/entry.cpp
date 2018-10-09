#include "../Core.h"
#include "../Scene.h"
#include "PlayerComponent.h"
#include "AIComponent.h"
using namespace fv;
using namespace game;

extern "C"
{
    GameObject* g;

    FV_DLL void entry()
    {
        g = NewGameObject();
        g->addComponent<PlayerComponent>()->addComponent<AIComponent>();
    }
}
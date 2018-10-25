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
        for ( u32 i =0; i <3; i ++)
        {
            g = NewGameObject();
            g->addComponent<PlayerComponent>()->addComponent<AIComponent>();
            
        }

        sceneManager()->addScene( "FirstScene" ).serialize( "myscene", true );
    }
}
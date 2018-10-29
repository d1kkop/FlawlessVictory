#include "../Core.h"
#include "../Scene.h"
#include "PlayerComponent.h"
#include "AIComponent.h"
using namespace fv;
using namespace game;

extern "C"
{
    GameObject* g;

    FV_DLL void entry(i32 argc, char** argv)
    {
        if ( argc > 2 )
        {
            if ( strcmp(argv[2],"save")==0)
            {
                u64 sceneMask = sceneManager()->addScene( "MainScene" );
                for ( u32 i =0; i <3; i ++)
                {
                    g = NewGameObject( sceneMask );
                    g->addComponent<PlayerComponent>()->addComponent<AIComponent>();
            
                }
                sceneManager()->serialize( "MainScene", "myscene", true );
            }
        }
        else
        {

            u64 sceneMask = sceneManager()->addScene("MainScene");
            sceneManager()->serialize("MainScene", "myscene", false);
            // save back
            sceneManager()->serialize("MainScene", "myscene", true);
        }
    }
}
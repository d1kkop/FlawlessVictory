#include "SystemManager.h"
#include "../Core/Functions.h"
#include "../Core/TypeManager.h"
#include "../Core/LogManager.h"
#include "../Scene/GameObject.h"
using namespace fv;


void shutdown()
{
    delete gameObjectManager();
    delete systemManager();
    delete typeManager();
    delete logManager();
}


int main(int argc, char** argv)
{
    if ( argc < 2 )
    {
        LOGC( "No module name specified." );
        return 0;
    }

    SystemParams params;
    params.moduleName = argv[1];

    if ( systemManager()->initialize( params ) )
    {
        systemManager()->mainloop();
    }

    shutdown();
    return 0;
}
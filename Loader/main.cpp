#include "../Core.h"
#include "../Scene.h"
#include "../Render.h"
#include "../System.h"
#include "../Core/InputManager.h"
using namespace fv;

#define FV_LOAD_TEST_MODULE 0

void shutdown()
{
    deleteJobManager();
    deleteInputManager();
    deleteGameObjectManager();
    deleteSystemManager();
    deleteTypeManager();
    deleteLogManager();
}


int main(int argc, char** argv)
{
    LOG("--- New Session ---");
    LOG("--- %s ---", LocalTime().c_str());
    LOG("-------------------");

    if ( argc < 2 )
    {
        LOGC("No module name specified.");
        deleteLogManager();
        return 0;
    }

    Path dllPath = Path(argv[0]);
#if !FV_LOAD_TEST_MODULE
    dllPath.remove_filename().append(argv[1]);
#else
    dllPath.remove_filename().append("Tests");
#endif

    SystemParams params{};
    params.moduleName = dllPath.string();
    params.windowWidth  = 1600;
    params.windowHeight = 900;
    params.fullscreen   = false;

    if ( systemManager()->initialize( params ) )
    {
    #if FV_LOAD_TEST_MODULE 
        Component* c = typeManager()->typeInfo("UnitTestComponent").createFunc(1);
        c->begin();
        return 0;
    #endif

        systemManager()->mainloop();
    }

    shutdown();
    return 0;
}
#include "../Core/TypeManager.h"
#include "../Core/LogManager.h"
#include "../Core/JobManager.h"
#include "../Core/InputManager.h"
#include "../Core/Functions.h"
#include "../Render/RenderManager.h"
#include "../Resource/ResourceManager.h"
#include "../Scene/GameObject.h"
#include "../System/SystemManager.h"
using namespace fv;

#define FV_LOAD_TEST_MODULE 0

void shutdown()
{
    deleteJobManager();
    deleteInputManager();
    deleteGameObjectManager();
    deleteSystemManager();
    deleteResourceManager();
    deleteRenderManager();
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
    dllPath.remove_filename().append("UnitTests");
#endif

    // Force cache all search directories, so that lookup is thread safe 
    resourceManager();
    
    SystemParams params{};
    params.moduleName = dllPath.string();
    params.windowWidth  = 1600;
    params.windowHeight = 900;
    params.fullscreen   = false;

    if ( systemManager()->initialize( params ) )
    {
    #if FV_LOAD_TEST_MODULE 
        GameComponent* c = (GameComponent*)typeManager()->typeInfo("UnitTestComponent")->createFunc(1);
        systemManager()->callBeginForComponent( c );
        return 0;
    #endif

        systemManager()->mainloop();
    }

    shutdown();
    return 0;
}
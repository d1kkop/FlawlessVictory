#include "../Core/TypeManager.h"
#include "../Core/LogManager.h"
#include "../Core/JobManager.h"
#include "../Core/InputManager.h"
#include "../Core/Functions.h"
#include "../Core/GameObject.h"
#include "../Core/ComponentManager2.h"
#include "../Core/TransformManager.h"
#include "../Render/RenderManager.h"
#include "../Resource/ResourceManager.h"
#include "../Resource/ShaderCompiler.h"
#include "../Resource/TextureImporter.h"
#include "../Resource/ModelImporter.h"
#include "../Resource/PatchManager.h"
#include "../System/SystemManager.h"
using namespace fv;

#define FV_LOAD_TEST_MODULE 0


void shutdown()
{
    SetEngineClosing();
    // delete all managers
    deleteResourceManager();
    deleteJobManager();
    deletePatchManager();
    // Delete components and game object manager so that refcount of resources is deduced.
    deleteComponentManager();
    deleteComponentManager2();
    deleteGameObjectManager();
    deleteShaderCompiler();
    deleteTextureImporter();
    deleteModelImporter();
    deleteRenderManager();
    // Do less tightly coupled manager
    deleteInputManager();
    deleteSystemManager();
    deleteTypeManager();
    deleteTransformManager();
    // At last log manager when there is no logging required anymore.
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
    
    SystemParams sysParams{};
    sysParams.moduleName   = dllPath.string();
    sysParams.argc = argc;
    sysParams.argv = argv;

    if ( systemManager()->initialize( sysParams ) )
    {
    #if !FV_LOAD_TEST_MODULE 
        systemManager()->mainloop();
    #endif
    }

    shutdown();
    return 0;
}
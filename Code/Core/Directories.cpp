#include "PCH.h"
#include "Directories.h"
namespace fs = std::experimental::filesystem;

namespace fv
{
    // Root
    Path g_Bin;
    Path g_Assets;
    Path g_Build;
    Path g_Intermediate;
    Path g_Log;
    Path g_Standard;
    // Intermediate
    Path g_IntermediateShaders;
    Path g_IntermediateMeshes;
    Path g_IntermediateTextures;

    const Path& Directories::bin() { return g_Bin; }
    const Path& Directories::assets() { return g_Assets; }
    const Path& Directories::build() { return g_Build; }
    const Path& Directories::intermediate() { return g_Intermediate; }
    const Path& Directories::log() { return g_Log; }
    const Path& Directories::standard() { return g_Standard; }
    const Path& Directories::intermediateShaders() { return g_IntermediateShaders; }
    const Path& Directories::intermediateMeshes() { return g_IntermediateMeshes; }
    const Path& Directories::intermediateTextures() { return g_IntermediateTextures; }

    
    u32 InitDirectories()
    {
        g_Bin = "Bin";
        g_Assets = "Assets";
        g_Build = "Build";
        g_Intermediate = "Intermediate/GeneratedAssets";
        g_Log = "Log";
        g_Standard = "Standard";
        g_IntermediateShaders  = g_Intermediate / "Shaders";
        g_IntermediateMeshes   = g_Intermediate / "Meshes";
        g_IntermediateTextures = g_Intermediate / "Textures";


    #if !FV_BUILD
        Path prefix = "../../";
        g_Bin = prefix / g_Bin;
        g_Assets = prefix / g_Assets;
        g_Build = prefix / g_Build;
        g_Log = prefix / g_Log;
        g_Intermediate = prefix / g_Intermediate;
        g_Standard = prefix / g_Standard;
        g_IntermediateShaders = prefix / g_IntermediateShaders;
        g_IntermediateMeshes = prefix / g_IntermediateMeshes;
        g_IntermediateTextures = prefix / g_IntermediateTextures;
    #endif

        fs::create_directory( g_Bin );
        fs::create_directory( g_Assets );
        fs::create_directory( g_Build );
        fs::create_directory( g_Log );
        fs::create_directory( g_Intermediate );
        fs::create_directory( g_Standard );
        fs::create_directory( g_IntermediateShaders );
        fs::create_directory( g_IntermediateMeshes );
        fs::create_directory( g_IntermediateTextures );

        return 1;
    }

    static u32 g_InitializeDirs = InitDirectories();
}
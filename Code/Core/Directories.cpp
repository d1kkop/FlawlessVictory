#include "PCH.h"
#include "Directories.h"
namespace fs = std::experimental::filesystem;

namespace fv
{
    Path g_Bin;
    Path g_Assets;
    Path g_Build;
    Path g_Intermediate;
    Path g_Log;
    Path g_Standard;
    Path g_IntermediateShaders;

    const Path& Directories::bin() { return g_Bin; }
    const Path& Directories::assets()  { return g_Assets; }
    const Path& Directories::build() { return g_Build; }
    const Path& Directories::intermediate() { return g_Intermediate; }
    const Path& Directories::log() { return g_Log; }
    const Path& Directories::standard() { return g_Standard; }
    const Path& Directories::intermediateShaders() { return g_IntermediateShaders; }

    
    u32 InitDirectories()
    {
        g_Bin = "Bin";
        g_Assets = "Assets";
        g_Build = "Build";
        g_Intermediate = "Intermediate";
        g_Log = "Log";
        g_Standard = "Standard";
        g_IntermediateShaders = g_Intermediate.append("Shaders");

    #if !FV_BUILD
        Path prefix = "../../";
        g_Bin = prefix / g_Bin;
        g_Assets = prefix / g_Assets;
        g_Build = prefix / g_Build;
        g_Log = prefix / g_Log;
        g_Intermediate = prefix / g_Intermediate;
        g_Standard = prefix / g_Standard;
        g_IntermediateShaders = prefix / g_IntermediateShaders;
    #endif

        fs::create_directory( g_Bin );
        fs::create_directory( g_Assets );
        fs::create_directory( g_Build );
        fs::create_directory( g_Log );
        fs::create_directory( g_Intermediate );
        fs::create_directory( g_Standard );
        fs::create_directory( g_IntermediateShaders );

        return 1;
    }

    static u32 g_InitializeDirs = InitDirectories();
}
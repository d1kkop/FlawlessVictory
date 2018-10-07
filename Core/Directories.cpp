#include "PCH.h"
#include "Directories.h"

namespace fv
{
    Path g_Assets;
    Path g_Build;
    Path g_Intermediate;
    Path g_Log;

    Path Directories::assets()  { return g_Assets; }
    Path Directories::build() { return g_Build; }
    Path Directories::intermediate() { return g_Intermediate; }
    Path Directories::log() { return g_Log; }

    
    u32 InitDirectories()
    {
        g_Assets = "Assets";
        g_Build = "Build";
        g_Intermediate = "Intermediate";
        g_Log = "Log";

    #if !FV_BUILD
        Path prefix = "../../";
        g_Assets = prefix / g_Assets;
        g_Build = prefix / g_Build;
        g_Log = prefix / g_Log;
        g_Intermediate = prefix / g_Intermediate;
    #endif
        return 1;
    }

    static u32 g_InitializeDirs = InitDirectories();
}
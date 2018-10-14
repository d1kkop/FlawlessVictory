#include "Assets.h"
#include "../Core/Directories.h"
#include "../Core/LogManager.h"

namespace fv
{
    // Extensions
    Path g_ImportExt;
    Path g_MeshBinExt;
    Path g_Tex2DBinExt;
    Path g_ShaderBinExt;

    const Path& Assets::importExtension() { return g_ImportExt; }
    const Path& Assets::meshBinExtension() { return g_MeshBinExt; }
    const Path& Assets::tex2DBinExtension() { return g_Tex2DBinExt; }
    const Path& Assets::shaderBinExtension() { return g_ShaderBinExt; }

    i32 InitAssets()
    {
        g_ImportExt = ".import";
        g_MeshBinExt = ".mesh";
        g_Tex2DBinExt = ".tex2d";
        g_ShaderBinExt = ".spv";
        return 1;
    }

    static i32 g_InitAssets = InitAssets();
}
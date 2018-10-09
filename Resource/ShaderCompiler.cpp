#include "ShaderCompiler.h"
#include "../Core/Functions.h"
#if FV_GLSL2SPIRVLIB
    #include "ShaderCompilerGLSL2SpirVLib.h"
    #include <shaderc/shaderc.hpp>
#elif FV_GLSL2SPIRVBIN
    #include "ShaderCompilerGLSL2SpirVBin.h"
#endif

namespace fv
{
    ShaderCompiler* g_ShaderCompiler {};
    ShaderCompiler* shaderCompiler()
    { 
    #if FV_GLSL2SPIRVLIB
        auto glsl2spirv = (ShaderCompilerGLSL2SpirVLib*)g_ShaderCompiler;
        return CreateOnce<ShaderCompilerGLSL2SpirVLib>( glsl2spirv );
    #elif FV_GLSL2SPIRVBIN
        auto glsl2spirv = (ShaderCompilerGLSL2SpirVBin*)g_ShaderCompiler;
        return CreateOnce<ShaderCompilerGLSL2SpirVBin>(glsl2spirv);
    #else
    #error no implementation
    #endif
        assert(false);
        return nullptr;
    }
    void deleteShaderCompiler() { delete g_ShaderCompiler; g_ShaderCompiler=nullptr; }

}
#include "ShaderCompiler.h"
#include "../Core/Functions.h"
#if FV_GLSL2SPIRV
    #include "ShaderCompilerGLSL2SpirV.h"
    #include <shaderc/shaderc.hpp>
#endif

namespace fv
{
    ShaderCompiler* g_ShaderCompiler {};
    ShaderCompiler* shaderCompiler()
    { 
    #if FV_GLSL2SPIRV
        auto glsl2spirv = (ShaderCompilerGLSL2SpirV*)g_ShaderCompiler;
        return CreateOnce<ShaderCompilerGLSL2SpirV>( glsl2spirv );
    #endif
        assert(false);
        return nullptr;
    }
    void deleteShaderCompiler() { delete g_ShaderCompiler; g_ShaderCompiler=nullptr; }
}
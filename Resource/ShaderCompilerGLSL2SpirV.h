#pragma once
#if FV_GLSL2SPIRV
#include "ShaderCompiler.h"
#include <shaderc/shaderc.hpp>

namespace fv
{
    class ShaderCompilerGLSL2SpirV: public ShaderCompiler
    {
    public:
        bool compileShader(const Path& pathIn, Vector<u32>& code) override;

    private:
        bool preprocessShader(const String& filename, shaderc_shader_kind kind, const String& source, String& preprocessed);
        bool compileFileToAssembly(const String& filename, shaderc_shader_kind kind, const String& source, bool optimize, String& assembly);
        bool compileFile(const String& filename, shaderc_shader_kind kind, const String& source, bool optimize, Vector<u32>& code);
        shaderc_shader_kind shaderKindFromExtension(const char* ext, bool valid);
    };
}
#endif

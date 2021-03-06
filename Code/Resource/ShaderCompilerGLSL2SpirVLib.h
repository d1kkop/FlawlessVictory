#pragma once
#include "ShaderCompiler.h"
#if FV_GLSL2SPIRVLIB
#include <shaderc/shaderc.hpp>

namespace fv
{
    class ShaderCompilerGLSL2SpirVLib: public ShaderCompiler
    {
    public:
        bool compileShader(const Path& pathIn, Vector<char>& code) override;

    private:
        bool preprocessShader(const String& filename, shaderc_shader_kind kind, const String& source, String& preprocessed);
        bool compileFileToAssembly(const String& filename, shaderc_shader_kind kind, const String& source, bool optimize, String& assembly);
        bool compileFile(const String& filename, shaderc_shader_kind kind, const String& source, bool optimize, Vector<char>& code);
        shaderc_shader_kind shaderKindFromExtension(const String& ext, bool& valid);
    };
}
#endif

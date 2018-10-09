#pragma once
#include "ShaderCompiler.h"
#if FV_GLSL2SPIRVBIN
#include <shaderc/shaderc.hpp>

namespace fv
{
    class ShaderCompilerGLSL2SpirVBin: public ShaderCompiler
    {
    public:
        Path replaceWithBinaryExtension( const Path& pathIn ) override;
        bool compileShader(const Path& pathIn, Vector<char>& code) override;
    };
}
#endif

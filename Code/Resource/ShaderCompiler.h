#pragma once
#include "../Core/Common.h"

namespace fv
{
    class ShaderCompiler
    {
    public:
        virtual bool compileShader(const Path& pathIn, Vector<char>& code) = 0;
    };

    FV_DLL ShaderCompiler* shaderCompiler();
    FV_DLL void deleteShaderCompiler();
}
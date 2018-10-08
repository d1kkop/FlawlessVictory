#include "ShaderCompilerGLSL2SpirV.h"
#if FV_GLSL2SPIRV
#include <fstream>
using namespace std;
using namespace shaderc;

namespace fv
{
    bool ShaderCompilerGLSL2SpirV::preprocessShader(const String& filename, shaderc_shader_kind kind, const String& source, String& preprocessed)
    {
        Compiler compiler;
        CompileOptions options;

        // Example: Like -DMY_DEFINE=1
        // options.AddMacroDefinition("MY_DEFINE", "1");

        PreprocessedSourceCompilationResult result = compiler.PreprocessGlsl(source, kind, filename.c_str(), options);

        if ( result.GetCompilationStatus() != shaderc_compilation_status_success )
        {
            LOGW("Shader preprocess error.\n%s.", result.GetErrorMessage().c_str());
            return false;
        }

        preprocessed = { result.cbegin(), result.cend() };
        return true;
    }

    bool ShaderCompilerGLSL2SpirV::compileFileToAssembly(const String& filename, shaderc_shader_kind kind, const String& source, bool optimize, String& assembly)
    {
        Compiler compiler;
        CompileOptions options;

        // Like -DMY_DEFINE=1
      //   options.AddMacroDefinition("MY_DEFINE", "1");
        if ( optimize ) options.SetOptimizationLevel(shaderc_optimization_level_performance);

        AssemblyCompilationResult result = compiler.CompileGlslToSpvAssembly( source, kind, filename.c_str(), options );

        if ( result.GetCompilationStatus() != shaderc_compilation_status_success )
        {
            LOGW("Shader compile file to assembly error.\n%s.", result.GetErrorMessage().c_str());
            return false;
        }

        assembly = { result.cbegin(), result.cend() };
        return true;
    }

    bool ShaderCompiler::compileFile(const String& filename, shaderc_shader_kind kind, const String& source, bool optimize, Vector<u32>& code)
    {
        Compiler compiler;
        CompileOptions options;

        // Like -DMY_DEFINE=1
       //  options.AddMacroDefinition("MY_DEFINE", "1");
        if ( optimize ) options.SetOptimizationLevel(shaderc_optimization_level_performance);

        SpvCompilationResult result = compiler.CompileGlslToSpv(source, kind, filename.c_str(), options);

        if ( result.GetCompilationStatus() != shaderc_compilation_status_success )
        {
            LOGW("Shader compile file binary error.\n%s.", result.GetErrorMessage().c_str());
            return false;
        }

        code = { module.cbegin(), module.cend() };
        return true;
    }

    shaderc_shader_kind ShaderCompilerGLSL2SpirV::shaderKindFromExtension(const String& ext, bool& valid)
    {
        valid = true;
        if ( ext == ".frag" )      return shaderc_glsl_default_fragment_shader;
        else if ( ext == ".vert" )      return shaderc_glsl_default_vertex_shader;
        else if ( ext == ".comp" )      return shaderc_glsl_default_compute_shader;
        else if ( ext == ".geom" )      return shaderc_glsl_default_geometry_shader;
        else if ( ext == ".tesa" )      return shaderc_glsl_default_tess_evaluation_shader;
        else if ( ext == ".tesc" )      return shaderc_glsl_default_tess_control_shader;
        valid = false;
        return shaderc_glsl_default_vertex_shader;
    }

    String ShaderCompilerGLSL2SpirV::findBinaryFile(const Path& pathIn)
    {
        Path newPath = pathIn.replace_extension(".spv");
        return resourceManager()->findBinaryFile( newPath );
    }

    bool ShaderCompilerGLSL2SpirV::compileShader(const Path& path, Vector<u32>& code)
    {
        // First check if spirV equivelent is already on disk
        ifstream file(path.replace_extension(".spv"), std::ios::ate, std::ios::binary);
        if ( file.is_open() )
        {
            size_t fileSize = file.tellg();
            file.seekg(0); // Go to beginning
            code.resize((fileSize+3)/4);
            file.read(buffer.data(), fileSize);
            file.close();
            // Loaded from binary
            return true;
        }

        String ext = path.extension().string();
        if ( !(ext == ".frag" || ext == ".vert" || ext == ".comp" || ext == ".geom" || ext == ".tesa" || ext == ".tesc") )
        {
            LOGW("Unknown shader extension %s.", ext.c_str());
            return;
        }

        bool validShaderKind;
        auto shaderKind = shaderKindFromExtension( ext, validShaderKind );
        if ( !validShaderKind )
        {
            LOGW("Cannot determine shader kind from extension %s. Failed to load shader %s.", ext.c_str(), path.string().c_str());
            return;
        }

        file.open(path.string().c_str(), std::ios::ate); // ate: Set to end
        if ( !file.is_open() )
        {
            LOGW("Cannot load file %s.", path.string().c_str());
            return;
        }

        size_t fileSize = file.tellg();
        file.seekg(0); // Go to beginning
        String buffer;
        buffer.resize(fileSize);
        file.read(buffer.data(), fileSize);
        file.close();

        code = compileFile( path.filename(), shaderKind, buffer, true );
        return true;
    }
}
#endif
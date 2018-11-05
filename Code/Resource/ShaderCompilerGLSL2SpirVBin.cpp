#include "ShaderCompilerGLSL2SpirVBin.h"
#if FV_GLSL2SPIRVBIN
#include "Assets.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
#include "../Core/OSLayer.h"
#include "../Core/Directories.h"

namespace fv
{
    bool ShaderCompilerGLSL2SpirVBin::compileShader(const Path& path, Vector<char>& code)
    {
        String ext = path.extension().string();
        if ( !(ext == ".frag" || ext == ".vert" || ext == ".comp" || ext == ".geom" || ext == ".tesa" || ext == ".tesc") )
        {
            LOGW("Unknown shader extension %s.", ext.c_str());
            return false;
        }

        Path output = Directories::intermediateShaders() / path.filename();
        output.replace_extension( ext + Assets::shaderBinExtension().string() );

        // https://vulkan.lunarg.com/doc/view/1.0.8.0/windows/spirv_toolchain.html
        String args;
        args += " -V"; // Create spirV binary Vulkan output
        args += " " + path.string();
        args += " -o " + output.string(); // Output file
        args += " -t"; // Multithreaded compilation
        OSHandle hProgram = OSStartProgram( (Directories::bin() / Path("glslangValidator.exe")).string().c_str(), args.c_str() );
        if ( hProgram.process )
        {
            OSWaitOnProgram( hProgram );
        }
        else 
        {
            LOGW("Failed to execute glsl to spirv binary.");
            return false;
        }

        return LoadBinaryFile(output.string().c_str(), code);
    }
}
#endif
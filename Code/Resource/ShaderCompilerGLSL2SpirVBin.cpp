#include "ShaderCompilerGLSL2SpirVBin.h"
#if FV_GLSL2SPIRVBIN
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
#include "../Core/OSLayer.h"
#include "../Core/Directories.h"

namespace fv
{

    Path ShaderCompilerGLSL2SpirVBin::replaceWithBinaryExtension(const Path& pathIn)
    {
        return Path(pathIn.string() + ".spv");
    }

    bool ShaderCompilerGLSL2SpirVBin::compileShader(const Path& path, Vector<char>& code)
    {
        String ext = path.extension().string();
        if ( !(ext == ".frag" || ext == ".vert" || ext == ".comp" || ext == ".geom" || ext == ".tesa" || ext == ".tesc") )
        {
            LOGW("Unknown shader extension %s.", ext.c_str());
            return false;
        }

        Path output = replaceWithBinaryExtension( Directories::intermediateShaders() / path.filename() );

        // https://vulkan.lunarg.com/doc/view/1.0.8.0/windows/spirv_toolchain.html
        String args;
        args += " -V"; // Create spirV binary Vulkan output
        args += " -o " + output.string(); // Output file
        args += " -t"; // Multithreaded compilation
        OSStartProgram( (Directories::bin() / Path("glslangValidator.exe")).string().c_str(), args.c_str(), true );

        return LoadBinaryFile(path.string().c_str(), code);
    }
}
#endif
#include "Shader.h"
#include "ResourceManager.h"
#include "ShaderCompiler.h"
#include "../Core/Functions.h"
#include "../Core/JobManager.h"
#include "../Core/Directories.h"
#include "../Render/RenderManager.h"
#include "../Render/GraphicResource.h"

namespace fv
{
    FV_TYPE_IMPL(Shader)

    Shader::~Shader()
    {
        renderManager()->freeGraphic(m_Graphic);
    }

    void Shader::load(const Path& path)
    {
        Vector<char> code;

        // Check if compiled file is already there
        Path binPath = shaderCompiler()->replaceWithBinaryExtension( Directories::intermediateShaders() / path.filename() );
        if ( !LoadBinaryFile(binPath.string().c_str(), code))
        {
            // Compile from glsl
            if ( !shaderCompiler()->compileShader(path, code) )
            {
                LOGW("Failed to load shader %s.", path.string().c_str());
            }
        }

        if ( !code.empty() )
        {
            m_Graphic = renderManager()->createGraphic<Shader>();
            if ( m_Graphic->updateShaderCode(code) )
            {
                m_LoadSuccesful = true;
            }
        }
    }

    void Shader::onDoneOrCancelled(class Job* j)
    {
        j->waitAndFree();
        m_LoadDone = true;
    }

}
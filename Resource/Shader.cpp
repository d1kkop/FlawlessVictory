#include "Shader.h"
#include "ResourceManager.h"
#include "ShaderCompiler.h"
#include "../Core/JobManager.h"
#include "../Core/Directories.h"
#include "../Render/RenderManager.h"
#include "../Render/GraphicResource.h"
#include <fstream>
using namespace std;

namespace fv
{
    FV_TYPE_IMPL(Shader)

    Shader::~Shader()
    {
        renderManager()->freeGraphic(m_Graphic);
    }

    void Shader::load(const Path& path)
    {
        if ( !path.has_extension() )
        {
            LOGW("Cannot load shader %s. No extension found.", path.string().c_str());
            return;
        }
    
        Vector<u32> code;
        if ( shaderCompiler()->compileShader( path, code ) )
        {
            m_Graphic = renderManager()->createGraphic<Shader>();
            m_Graphic->updateShaderCode( code );
            m_LoadSuccesful = true;
        }
    }

    void Shader::onDoneOrCancelled(class Job* j)
    {
        j->waitAndFree();
        m_LoadDone = true;
    }

}
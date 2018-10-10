#include "Shader.h"
#include "ResourceManager.h"
#include "ShaderCompiler.h"
#include "../Core/Functions.h"
#include "../Core/Directories.h"
#include "../Core/LogManager.h"
#include "../Render/RenderManager.h"
#include "../Render/GraphicResource.h"
#include "../Resource/PatchManager.h"

namespace fv
{
    FV_TYPE_IMPL(Shader)

    Shader::~Shader()
    {
        renderManager()->freeGraphic(m_Graphic, true);
    }

    void Shader::applyPatch(GraphicResource* graphic)
    {
        FV_CHECK_MO();

        if ( m_Graphic )
            renderManager()->freeGraphic( m_Graphic );

        m_Graphic = graphic;
    }

    void Shader::load(const ResourceToLoad& rtl)
    {
        Vector<char> code;

        // Check if compiled file is already there
        Path binPath = shaderCompiler()->replaceWithBinaryExtension( Directories::intermediateShaders() / rtl.loadPath.filename() );
        if ( rtl.reload || !LoadBinaryFile(binPath.string().c_str(), code) )
        {
            code.clear(); // In case binary load partially succeeded
            // Compile from glsl
            if ( !shaderCompiler()->compileShader(rtl.loadPath, code) )
            {
                LOGW("Failed to load shader %s.", rtl.loadPath.string().c_str());
            }
        }

        if ( !code.empty() )
        {
            GraphicResource* graphic = renderManager()->createGraphic(GraphicType::Shader);
            if ( graphic )
            {
                if ( graphic->updateShaderCode( code ) )
                {
                    Patch* p = patchManager()->createPatch(PatchType::ShaderCode);
                    p->graphic  = graphic;
                    p->resource = rtl.resource;
                    patchManager()->submitPatch( p );
                }
                else
                {
                    renderManager()->freeGraphic( graphic, true );
                }
            }
        }
    }

}
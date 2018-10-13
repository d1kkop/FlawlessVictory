#include "Patch.h"
#include "../Core/Thread.h"
#include "../Resource/Texture2D.h"
#include "../Resource/Shader.h"
#include "../Render/RenderManager.h"

namespace fv
{
    Patch::~Patch()
    {
        // Graphic is only valid ptr if patch was not applied. Eg at end of application.
        // RenderManager will also clean up this graphic if not cleaned from here.
        if ( graphic )
        {
            renderManager()->freeGraphic( graphic );
        }
    }

    void Patch::applyPatch()
    {
        switch ( patchType )
        {
        case PatchType::Texture2DData:
            applyTexture2DLoad();
            break;
        case PatchType::ShaderCode:
            applyShaderCode();
            break;
        }
    }

    void Patch::applyTexture2DLoad()
    {
        FV_CHECK_MO();
        M<Texture2D> tex = std::static_pointer_cast<Texture2D>( resource );
        tex->applyPatch( width, height, imgFormat, graphic );
        graphic = nullptr; // Ownership transfered.
    }

    FV_MO void Patch::applyShaderCode()
    {
        FV_CHECK_MO();
        M<Shader> shader = std::static_pointer_cast<Shader>(resource);
        shader->applyPatch( graphic );
        graphic = nullptr; // Ownership transfered.
    }

}
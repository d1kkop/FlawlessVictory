#include "Patch.h"
#include "PatchManager.h"
#include "../Core/Thread.h"
#include "../Resource/Texture2D.h"
#include "../Resource/Shader.h"
#include "../Resource/Mesh.h"
#include "../Render/RenderManager.h"

namespace fv
{
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
        case PatchType::MeshData:
            applyMeshData();
            break;
        }
    }

    void Patch::submit()
    {
        patchManager()->submitPatch( this );
    }

    void Patch::applyTexture2DLoad()
    {
        FV_CHECK_MO();
        M<Texture2D> tex = spc<Texture2D>( resource );
        assert(tex);
     //   tex->applyPatch( width, height, imgFormat, texture2D );
        //texture2D = {}; // Ownership transferred.
    }

    FV_MO void Patch::applyShaderCode()
    {
        FV_CHECK_MO();
        M<Shader> shader = spc<Shader>(resource);
        assert(shader);
        shader->setDeviceShader( graphic );
    }

    FV_MO void Patch::applyMeshData()
    {
        FV_CHECK_MO();
        //M<Mesh> mesh = spc<Mesh>(resource);
        //assert(mesh);
        //mesh->applyPatch( submeshes, hostMeshes, materials );
        //// Should all have been moved
        //submeshes.clear();
        //hostMeshes.clear();
        //materials.clear();
    }

}
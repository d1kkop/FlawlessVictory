#include "Mesh.h"
#include "ModelImporter.h"
#include "ResourceManager.h"
#include "PatchManager.h"
#include "../Core/Thread.h"
#include "../Render/RenderManager.h"

namespace fv
{
    FV_TYPE_IMPL(Mesh)

    Mesh::~Mesh()
    {
        for ( auto* gr : m_SubMeshes )
            renderManager()->freeGraphic( gr );
    }

    void Mesh::applyPatch(u32 numVertices, u32 numIndices, const Vector<GraphicResource*>& submeshes)
    {
        FV_CHECK_MO();

        // free old
        for ( auto* gr : m_SubMeshes )
            renderManager()->freeGraphic( gr );

        m_SubMeshes = std::move(submeshes);
    }

    void Mesh::applySubmeshPatch(u32 subMeshIdx, const float** vertexData, const u32* numComponents, u32 numAttachments)
    {
        FV_CHECK_MO();
    }

    void Mesh::load(const ResourceToLoad& rtl)
    {
        Vector<Submesh> newSubmeshes;
        if ( !modelImporter()->reimport( rtl.loadPath, newSubmeshes ) )
        {
            return;
        }

        if ( newSubmeshes.empty() )
        {
            LOGW("Failed to load %s.", rtl.loadPath.string().c_str());
            return;
        }

        Vector<GraphicResource*> meshGraphics;
        for ( auto& sm : newSubmeshes )
        {
            GraphicResource* graphic = renderManager()->createGraphic(GraphicType::Submesh, 0 /* device id */);
            if ( graphic && graphic->updateMeshData( sm ) )
            {
                meshGraphics.emplace_back( graphic );
            }
            else
            {
                LOGW("Failed to update one or more submeshes of mesh %s.", rtl.loadPath.string().c_str());
                renderManager()->freeGraphic( graphic );
            }
        }

        Patch* patch = patchManager()->createPatch( PatchType::MeshData );
        patch->submeshes = std::move( meshGraphics );
        patch->submit();
    }

}
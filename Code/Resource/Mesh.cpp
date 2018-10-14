#include "Mesh.h"
#include "Assets.h"
#include "ModelImporter.h"
#include "ResourceManager.h"
#include "PatchManager.h"
#include "../Core/Thread.h"
#include "../Core/Directories.h"
#include "../Render/RenderManager.h"

namespace fv
{
    FV_TYPE_IMPL(Mesh)

    Mesh::~Mesh()
    {
        for ( auto* gr : m_SubMeshes )
            renderManager()->freeGraphic( gr );
    }

    void Mesh::applyPatch(const Vector<GraphicResource*>& submeshes, const Vector<Submesh>& hostMeshes)
    {
        FV_CHECK_MO();

        // free old
        for ( auto* gr : m_SubMeshes )
            renderManager()->freeGraphic( gr );

        m_SubMeshes  = std::move(submeshes);
        m_HostMeshes = std::move(hostMeshes);
    }

    void Mesh::load(const ResourceToLoad& rtl)
    {
        Vector<Submesh> newSubmeshes;

        // Try get import settings from file
        MeshImportSettings settings {};
        bool importSettingsRead = settings.read(rtl.loadPath);

        if ( !importSettingsRead )
        {
            // Write import file
            if ( !settings.write(rtl.loadPath) )
            {
                LOGW("Failed to write import file for %s.", rtl.loadPath.string().c_str());
                // Continue though.
            }
        }
        
        // Try load from binary if no import is forced
        Path binPath = Directories::intermediateMeshes() / rtl.loadPath.filename();
        binPath.replace_extension(Assets::meshBinExtension());
        if ( rtl.reimport || !modelImporter()->loadBinary(binPath, newSubmeshes ) )
        {
            newSubmeshes.clear(); // In case binary load succeeded partially.
            if ( !modelImporter()->reimport(rtl.loadPath, settings, newSubmeshes) ||
                 newSubmeshes.empty() )
            {
                LOGW("Failed to load %s.", rtl.loadPath.string().c_str());
                return;
            }
        }

        // Create graphical component of each submesh
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

        // Create a patch to be applied on main thread with new graphical content.
        Patch* patch = patchManager()->createPatch( PatchType::MeshData );
        patch->submeshes = std::move( meshGraphics );
        patch->resource = rtl.resource;
        if ( settings.keepInRam )
        {
            patch->hostMeshes = std::move( newSubmeshes );
        }
        patch->submit();
    }

}
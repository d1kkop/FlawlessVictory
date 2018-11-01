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
        for ( auto sm : m_SubMeshes )
        {
            renderManager()->deleteSubmesh( sm );
        }
    }

    void Mesh::applyPatch(const Vector<RSubmesh>& submeshes, const Vector<Submesh>& hostMeshes, const Vector<M<Material>>& materials)
    {
        FV_CHECK_MO();

        for ( auto sm : m_SubMeshes )
        {
            renderManager()->deleteSubmesh( sm );
        }

        m_SubMeshes  = std::move(submeshes);
        m_HostMeshes = std::move(hostMeshes);
        m_Materials  = std::move(materials);
    }

    void Mesh::load(const ResourceToLoad& rtl)
    {
        Vector<Submesh> subMeshes;
        Vector<M<Material>> materials;
        u32 devIdx = renderManager()->autoDeviceIdx();

        // Try get import settings from file
        MeshImportSettings settings {};
        bool importSettingsRead = settings.read(rtl.loadPath);

        if ( !importSettingsRead )
        {
            // Write import file
            if ( !settings.write(rtl.loadPath) )
            {
                LOGW("Failed to write import file for %s.", rtl.loadPath.string().c_str()); // Continue though
            }
        }
        
        // Try load from binary if no import is forced
        Path binPath = Directories::intermediateMeshes() / rtl.loadPath.filename();
        binPath.replace_extension(Assets::meshBinExtension());
        if ( rtl.reimport || !modelImporter()->loadBinary(binPath, subMeshes) )
        {
            subMeshes.clear(); // In case binary load succeeded partially.
            if ( !modelImporter()->reimport(rtl.loadPath, settings, subMeshes, materials) ||
                 subMeshes.empty() )
            {
                LOGW("Failed to load %s (num submeshes %d).", rtl.loadPath.string().c_str(), (u32)subMeshes.size());
                return;
            }
        }

        // Create graphical component of each submesh
        Vector<RSubmesh> graphicSubmeshes;
        u32 i=0;
        for ( auto& sm : subMeshes )
        {
            SubmeshInput si;
            si.positions = true;
            si.normals = sm.normals.size();
            si.tanBins = sm.tangents.size() && sm.bitangents.size();
            si.uvs = sm.uvs.size();
            si.lightUvs = sm.lightUVs.size();
            si.extras[0] = sm.extra1.size();
            si.extras[1] = sm.extra2.size();
            si.extras[2] = sm.extra3.size();
            si.extras[3] = sm.extra4.size();
            si.bones = sm.weights.size() && sm.boneIndices.size();

            RSubmesh graphicSubmesh = renderManager()->createSubmesh( devIdx, sm, si );
            if ( !graphicSubmesh )
            {
                LOGW("Failed to update one or more submeshes of mesh %s. Complete update discarded.", rtl.loadPath.string().c_str());
                for ( auto& gsm : graphicSubmeshes )
                {
                    renderManager()->deleteSubmesh( gsm );
                }
                return;
            }
            graphicSubmeshes.emplace_back( graphicSubmesh );
        }

        // Create a patch to be applied on main thread with new graphical content.
        Patch* patch = patchManager()->createPatch( PatchType::MeshData );
        patch->submeshes = std::move( graphicSubmeshes );
        patch->materials = std::move( materials ); // Materials only available on first import
        patch->resource  = rtl.resource;
        if ( settings.keepInRam )
        {
            patch->hostMeshes = std::move( subMeshes );
        }
        patch->submit();
    }

    void Mesh::drawMT()
    {
        for ( auto& s : m_SubMeshes )
        {
            renderManager()->renderSubmesh( s );
        }
    }

}
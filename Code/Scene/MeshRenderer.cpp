#include "MeshRenderer.h"
#include "../Core/Thread.h"
#include "../Core/TextSerializer.h"
#include "../Resource/Mesh.h"
#include "../Resource/ResourceManager.h"

namespace fv
{
    FV_TYPE_IMPL(MeshRenderer)

    void MeshRenderer::setMesh(const M<Mesh> mesh)
    {
        m_Mesh = mesh;
        m_DeviceIdx = m_Mesh->deviceIdx();
    }

    M<Mesh>& MeshRenderer::mesh()
    {
        return m_Mesh;
    }

    void MeshRenderer::cullMT(u32 tIdx)
    {
        m_Culled = false; // TODO impl cull method
    }

    void MeshRenderer::drawMT(u32 tIdx)
    {
        if ( m_Culled ) return;
        if ( !m_Mesh ) return;
        m_Mesh->drawMT(tIdx);
    }

    void MeshRenderer::serialize(TextSerializer& ts)
    {
        FV_CHECK_MO();
        if ( ts.isWriting() )
        {
            if ( m_Mesh )
            {
                String filename = m_Mesh->filename();
                ts.serialize("filename", filename );
            }
        }
        else
        {
            String filename;
            ts.serialize("filename", filename);
            m_Mesh = resourceManager()->load<Mesh>( filename );
        }
    }

}
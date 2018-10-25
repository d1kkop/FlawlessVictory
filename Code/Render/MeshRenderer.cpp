#include "MeshRenderer.h"

namespace fv
{
    FV_TYPE_IMPL(MeshRenderer)

    void MeshRenderer::setMesh(const M<Mesh> mesh)
    {
        m_Mesh = mesh;
    }

    M<Mesh>& MeshRenderer::mesh()
    {
        return m_Mesh;
    }

    void MeshRenderer::cullMT()
    {
        m_Culled = false; // TODO impl cull method
    }

    void MeshRenderer::drawMT()
    {
        if ( m_Culled ) return;
        if ( !m_Mesh ) return;
        
        // Update materials.. ?
    }

    void MeshRenderer::serialize(TextSerializer& ts)
    {

    }

}
#pragma once
#include "Resource.h"
#include "Material.h"
#include "../Render/RenderManager.h"

namespace fv
{
    class Mesh: public Resource
    {
        FV_TYPE(Mesh)

    public:
        FV_DLL ~Mesh() override;
        FV_MO void applyPatch(const Vector<RSubmesh>& submeshes, const Vector<Submesh>& hostMeshes, const Vector<M<Material>>& materials);

    private:
        // From different thread
        FV_DLL void load_RT(const ResourceToLoad& rtl) override;
        FV_DLL void drawMT(u32 tIdx);

    private:
        Vector<RSubmesh> m_SubMeshes;
        Vector<M<Material>> m_Materials;
        Vector<Submesh> m_HostMeshes{}; // Usually empty

        friend class MeshRenderer;
    };

    //using Mesh = M<Mesh>;
}
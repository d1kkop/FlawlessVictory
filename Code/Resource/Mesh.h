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
        FV_MO void applyPatch(const Vector<u64>& submeshes, const Vector<Submesh>& hostMeshes, const Vector<M<Material>>& materials);

    private:
        // From different thread
        FV_DLL void load(const ResourceToLoad& rtl) override;

    private:
        Vector<u64> m_SubMeshes;
        Vector<M<Material>> m_Materials;
        Vector<Submesh> m_HostMeshes{}; // Usually empty
    };
}
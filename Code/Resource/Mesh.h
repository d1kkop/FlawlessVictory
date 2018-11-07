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
        FV_DLL u32 deviceIdx() const { return m_DeviceIdx; }

    private:
        // From different thread
        FV_DLL void load(const ResourceToLoad& rtl) override;
        FV_DLL void drawMT(u32 tIdx);

    private:
        u32 m_DeviceIdx = - 1;
        Vector<RSubmesh> m_SubMeshes;
        Vector<M<Material>> m_Materials;
        Vector<Submesh> m_HostMeshes{}; // Usually empty

        friend class MeshRenderer;
    };
}
#pragma once
#include "../Core/Common.h"
#include "Resource.h"
#include "../Render/GraphicResource.h"

namespace fv
{
    struct Submesh
    {
        Vector<u32>  indices;
        Vector<Vec3> vertices;
        Vector<Vec3> normals;
        Vector<Vec3> tangents;
        Vector<Vec3> bitangents;
        Vector<Vec2> uvs; 
        Vector<Vec2> lightUVs;
        Vector<Vec4> weights;
        Vector<byte> boneIndices;
        Vector<Vec4> extra1; 
        Vector<Vec4> extra2; 
        Vector<Vec4> extra3;
        Vector<Vec4> extra4;
        Vec3 bMin{};
        Vec3 bMax{};
    };

    class Mesh: public Resource
    {
        FV_TYPE(Mesh)

    public:
        FV_DLL ~Mesh() override;
        FV_MO void applyPatch(const Vector<GraphicResource*>& submeshes, const Vector<Submesh>& hostMeshes);

    private:
        // From different thread
        FV_DLL void load(const ResourceToLoad& rtl) override;

    private:
        Vector<GraphicResource*> m_SubMeshes;
        Vector<Submesh> m_HostMeshes{}; // Usually empty
    };
}
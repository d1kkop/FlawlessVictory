#pragma once
#include "../Render/RenderComponent.h"

namespace fv
{
    class Mesh;
    class TextSerializer;

    class MeshRenderer: public RenderComponent
    {
        FV_TYPE_FLAGS(MeshRenderer, FV_DRAW)

    public:
        FV_MO FV_DLL void setMesh(const M<Mesh> mesh);
        FV_MO FV_DLL M<Mesh>& mesh();

    protected:
        FV_DLL void cullMT(u32 tIdx) override;
        FV_DLL void drawMT(u32 tIdx) override;
        FV_MO FV_DLL void serialize( TextSerializer& ts ) override;

    private:
        M<Mesh> m_Mesh;
    };
}
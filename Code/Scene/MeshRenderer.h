#pragma once
#include "GameComponent.h"

namespace fv
{
    class Mesh;
    class TextSerializer;

    class MeshRenderer: public GameComponent
    {
        FV_TYPE_FLAGS(MeshRenderer, FV_DRAW)

    public:
        FV_MO FV_DLL void setMesh(const M<Mesh> mesh);
        FV_MO FV_DLL M<Mesh>& mesh();

    protected:
        FV_DLL void cullMT() override;
        FV_DLL void drawMT() override;
        FV_MO FV_DLL void serialize( TextSerializer& ts ) override;

    private:
        M<Mesh> m_Mesh;
        bool m_Culled;
    };
}
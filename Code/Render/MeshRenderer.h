#pragma once
#include "../Scene/GameComponent.h"

namespace fv
{
    class Mesh;

    class MeshRenderer: public GameComponent
    {
        FV_TYPE_FLAGS(MeshRenderer, FV_DRAW)

    public:
        FV_MO void setMesh(const M<Mesh> mesh);
        FV_MO M<Mesh>& mesh();

    protected:
        void cullMT() override;
        void drawMT() override;

    private:
        M<Mesh> m_Mesh;
        bool m_Culled;
    };
}
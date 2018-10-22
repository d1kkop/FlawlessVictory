#pragma once
#include "../Core.h"
#include "../Scene.h"
#include "../Resources.h"
using namespace fv;

namespace game
{
    class AIComponent: public GameComponent
    {
        FV_TYPE_FLAGS(AIComponent, FV_UPDATE | FV_DRAW)

    public:
        AIComponent()
        {
            m_UpdatePriority = 10;
        }

        void begin() override;
        void updateMT(float t) override;
        void update(float t) override;

    private:
        M<Shader> m_FragShader;
        M<Shader> m_VertShader;
        M<Mesh>   m_Sponza;
        M<Mesh>   m_Tyra;
        Ref<class PlayerComponent> m_TargetPlayer;
    };
}

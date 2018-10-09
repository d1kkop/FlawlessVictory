#pragma once
#include "../Core.h"
#include "../Scene.h"
#include "../Resources.h"
using namespace fv;

namespace game
{
    class AIComponent: public GameComponent
    {
        FV_TYPE(AIComponent)

    public:
        AIComponent()
        {
            m_UpdatePriority = 10;
            m_DoUpdate = true;
        }

        void begin() override;
        void updateMT(float t) override;
        void update(float t) override;

    private:
        M<Shader> m_FragShader;
        M<Shader> m_VertShader;
        Ref<class PlayerComponent> m_TargetPlayer;
    };
}

#pragma once
#include "../Core.h"
#include "../Scene.h"
using namespace fv;

namespace game
{
    class AIComponent: public GameComponent
    {
        FV_TYPE(AIComponent, 0, false)

    public:
        void begin() override;
        void updateMT(float t) override;
        void update(float t) override;

    private:
        Ref<class PlayerComponent> m_TargetPlayer;
    };
}

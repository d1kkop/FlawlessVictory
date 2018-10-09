#pragma once
#include "../Scene.h"
using namespace fv;

namespace game
{
    class PlayerComponent: public GameComponent
    {
        FV_TYPE(PlayerComponent)

    public:
        PlayerComponent()
        {
            m_DoUpdate = true;
        }
        void updateMT(float t) override;
        void update(float t) override;
    };
}

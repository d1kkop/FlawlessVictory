#pragma once
#include "../Scene.h"
using namespace fv;

namespace game
{
    class PlayerComponent: public Component
    {
        FV_TYPE(PlayerComponent, 0, false)

    public:
        void updateMT(float t) override;
        void update(float t) override;
    };
}

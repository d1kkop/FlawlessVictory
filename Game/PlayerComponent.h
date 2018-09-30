#pragma once
#include "../Core.h"
using namespace fv;

namespace game
{
    class PlayerComponent: public Component
    {
        FV_TYPE(PlayerComponent, 0, false)

    public:
        virtual void updateMT(float t) override;
        virtual void update(float t) override;
    };
}

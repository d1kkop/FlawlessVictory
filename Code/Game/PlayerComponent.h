#pragma once
#include "../Scene.h"
using namespace fv;

namespace game
{
    class PlayerComponent: public GameComponent
    {
        FV_TYPE_FLAGS(PlayerComponent, FV_UPDATE)

    public:
        PlayerComponent()
        {
        }
        void updateMT(float t) override;
        void update(float t) override;
    };
}

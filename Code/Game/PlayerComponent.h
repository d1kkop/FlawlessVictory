#pragma once
#include "../Scene.h"
using namespace fv;

namespace game
{

    class PlayerComponent: public GameComponent
    {
        FV_TYPE_FLAGS(PlayerComponent, FV_UPDATE)

  //  protected:
        void serialize( class fv::TextSerializer& ts ) override;

    public:
        void updateMT(float t) override;
        void update(float t) override;
    };
}

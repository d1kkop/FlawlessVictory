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

    protected:
        void serialize( class fv::TextSerializer& ts ) override;

    private:
        M<Mesh>   m_Sponza;
        M<Mesh>   m_Tyra;
        Ref<class PlayerComponent> m_TargetPlayer;
    };
}

#pragma once
#include "../Core/Component.h"

namespace fv
{
    class GameObject;
    class TextSerializer;

    class GameComponent: public Component
    {
    protected:
        FV_MO virtual void begin() { }
        FV_MO virtual void update(float dt) { }
        virtual void updateMT(float dt) { }

        i32  m_UpdatePriority = 0;

    private:
        bool m_HasBegun = false;

        friend class SystemManager;
    };
}
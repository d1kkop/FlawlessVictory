#pragma once
#include "Common.h"
#include "Reflection.h" // <-- Dont remove include

namespace fv
{
    class Component
    {
    public:
        virtual void updateMT(float dt) { }
        virtual void updateST(float dt) { }
        virtual void physicsUpdateMT(float dt) { }
        virtual void networkUpdateMT(float dt) { }
        virtual u32 updatePriority() const { return 0; }

        bool m_DoUpdate = false;
        bool m_DoPhysicsUpdate = false;
        bool m_DoNetworkUpdate = false;
    };
}
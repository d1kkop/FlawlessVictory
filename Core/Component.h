#pragma once
#include "Common.h"
#include "Reflection.h" // <-- Dont remove include

namespace fv
{
    class Component
    {
    public:
        FV_ST virtual void begin(float dt) { }
        FV_ST virtual void update(float dt) { }
        virtual void updateMT(float dt) { }
        virtual void physicsUpdateMT(float dt) { }
        virtual void networkUpdateMT(float dt) { }
        virtual u32 updatePriority() const { return 0; }
        bool isInFreeList() const { return m_InFreeList; }
        u32 type() const { return m_Type; }

        bool m_DoUpdate = false;
        bool m_DoPhysicsUpdate = false;
        bool m_DoNetworkUpdate = false;

    private:
        bool m_InFreeList = true;
        u32 m_Type = -1;

        friend class ComponentManager;
    };
}
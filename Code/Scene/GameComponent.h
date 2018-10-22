#pragma once
#include "../Core/Component.h"

namespace fv
{
    class GameComponent: public Component
    {
    public:
        FV_DLL GameComponent* addComponent(u32 type);
        FV_DLL GameComponent* getComponent(u32 type);
        FV_DLL bool hasComponent(u32 type);
        FV_DLL bool removeComponent(u32 type);
        template <class T> T* addComponent();
        template <class T> T* getComponent();
        template <class T> bool hasComponent();
        template <class T> bool removeComponent();

    private:
        virtual void begin() { }
        virtual void update(float dt) { }
        virtual void updateMT(float dt) { }
        virtual void physicsUpdateMT(float dt) { }
        virtual void networkUpdateMT(float dt) { }
        virtual void cullMT() { }
        virtual void drawMT() { }

    protected:
        bool m_HasBegun = false;
        i32  m_UpdatePriority = 0;

    private:
        class GameObject* m_GameObject {};

        friend class GameObject;
        friend class SystemManager;
    };


    template <class T>
    T* GameComponent::addComponent() { return sc<T*>(addComponent(T::type())); }
    template <class T>
    T* GameComponent::getComponent() { return sc<T*>(getComponent(T::type())); }
    template <class T>
    bool GameComponent::hasComponent() { return hasComponent(T::type); }
    template <class T>
    bool GameComponent::removeComponent() { return removeComponent(T::type()); }
}
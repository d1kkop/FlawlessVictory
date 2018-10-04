#pragma once
#include "../Core/Component.h"

namespace fv
{
    class GameComponent: public Component
    {
    public:
        u32 updatePriority() const { return m_UpdatePriority; }
        virtual bool updatable() const { return false; }

        FV_DLL GameComponent* addComponent(u32 type);
        FV_DLL GameComponent* getComponent(u32 type);
        FV_DLL bool hasComponent(u32 type);
        FV_DLL bool removeComponent(u32 type);
        template <class T> T* addComponent();
        template <class T> T* getComponent();
        template <class T> bool hasComponent();
        template <class T> bool removeComponent();

    private:
        FV_MO virtual void begin() { }
        FV_MO virtual void update(float dt) { }
        virtual void updateMT(float dt) { }
        virtual void physicsUpdateMT(float dt) { }
        virtual void networkUpdateMT(float dt) { }

        bool m_HasBegun = false;
        bool m_UpdatePriority;
        class GameObject* m_GameObject {};

        friend class SystemManager;
    };


    class UpdatableGameComponent: public GameComponent
    {
    public:
        bool updatable() const override { return true; }
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
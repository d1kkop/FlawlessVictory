#pragma once
#include "../Core/Common.h"

namespace fv
{
    class Component
    {
    public:
        virtual u32 updatePriority() const { return 0; }
        virtual bool updatable() const { return false; }
        u32 type() const { return m_Type; }

        FV_DLL Component* addComponent(u32 type);
        FV_DLL Component* getComponent(u32 type);
        FV_DLL bool hasComponent(u32 type);
        FV_DLL bool removeComponent(u32 type);
        template <class T> T* addComponent();
        template <class T> T* getComponent();
        template <class T> bool hasComponent();
        template <class T> bool removeComponent();

    private:
        FV_ST virtual void begin() { }
        FV_ST virtual void update(float dt) { }
        virtual void updateMT(float dt) { }
        virtual void physicsUpdateMT(float dt) { }
        virtual void networkUpdateMT(float dt) { }

        u32 m_Type = -1;
        class GameObject* m_GameObject{};

        friend class SystemManager;
        friend class ComponentManager;
    };


    class UpdatableComponent: public Component
    {
    public:
        bool updatable() const override { return true; }
    };


    template <class T>
    T* Component::addComponent()       { return sc<T*>(addComponent(T::type())); }
    template <class T>
    T* Component::getComponent()       { return sc<T*>(getComponent(T::type())); }
    template <class T>
    bool Component::hasComponent()     { return hasComponent(T::type); }
    template <class T>
    bool Component::removeComponent()  { return removeComponent(T::type()); }
}
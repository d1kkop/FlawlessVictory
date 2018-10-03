#pragma once
#include "../Core/Common.h"
#include "../Core/ObjectManager.h"

namespace fv
{
    class FV_DLL Component: public Object
    {
    public:
        virtual ~Component()= default;
        virtual u32 updatePriority() const { return 0; }
        virtual bool updatable() const { return false; }
        u32 type() const { return m_Type; }

        Component* addComponent(u32 type);
        Component* getComponent(u32 type);
        bool hasComponent(u32 type);
        bool removeComponent(u32 type);
        template <class T> T* addComponent();
        template <class T> T* getComponent();
        template <class T> bool hasComponent();
        template <class T> bool removeComponent();

    private:
    public: // TODO remove
        FV_MO virtual void begin() { }
        FV_MO virtual void update(float dt) { }
        virtual void updateMT(float dt) { }
        virtual void physicsUpdateMT(float dt) { }
        virtual void networkUpdateMT(float dt) { }

        u32  m_Type = -1;
        bool m_HasBegun = false;
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
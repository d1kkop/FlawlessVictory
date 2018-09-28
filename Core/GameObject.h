#pragma once
#include "Platform.h"
#include "Component.h"
#include "System.h"
#include "ComponentManager.h"

namespace fv
{
    class GameObject
    {
    public:
        template <class T> T*   addOrGetComponent();
        template <class T> bool hasComponent();
        template <class T> bool removeComponent();
        FV_DLL u32 numComponents();

    private:
        Map<u32, Component*> m_Components;
    };


    template <typename T>
    T* GameObject::addOrGetComponent()
    {
        auto cIt = m_Components.find(T::type());
        if ( cIt == m_Components.end() )
        {
            Component* c = componentManager()->newComponent<T>();
            m_Components[T::type()] = c;
            return sc<T*>(c);
        }
        return sc<T*>(cIt->second);
    }

    template <class T>
    bool GameObject::hasComponent()
    {
        return m_Components.count(T::type()) != 0;
    }

    template <class T>
    bool GameObject::removeComponent()
    {
        auto cIt = m_Components.find(T::type());
        if ( cIt != m_Components.end() )
        {
            componentManager()->freeComponent<T>(cIt->second);
            m_Components.erase(cIt);
            return true;
        }
        return false;
    }
}

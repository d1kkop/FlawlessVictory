#pragma once
#include "../Core/Platform.h"
#include "../Core/Component.h"
#include "../Core/ComponentManager.h"

namespace fv
{
    class GameObject
    {
    public:
        template <class T, class... Args> T* addComponent(Args... args);
        template <class T> T* getComponent();
        template <class T> bool hasComponent();
        template <class T> bool removeComponent();
        FV_DLL u32 numComponents();

    private:
        u32 m_Id = -1;
        Map<u32, Component*> m_Components;
    };


    template <class T, class... Args>
    T* GameObject::addComponent(Args... args)
    {
        auto cIt = m_Components.find(T::type());
        Component* pComponent;
        if ( cIt == m_Components.end() )
        {
            pComponent = componentManager()->newComponent<T>();
            m_Components[T::type()] = pComponent;
        }
        else pComponent = cIt->second;
        new (pComponent)T(args...); // Reinvoke the constructor
        return sc<T*>(pComponent);
    }

    template <class T>
    T* GameObject::getComponent()
    {
        auto cIt = m_Components.find(T::type());
        if ( cIt != m_Components.end() )
        {
            return sc<T*>(cIt->second);
        }
        return nullptr;
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

#pragma once
#include "Component.h"
#include "ComponentManager.h"
#include "../Core/ObjectManager.h"
#include "../Core/Platform.h"
#include "../Core/Thread.h"

namespace fv
{
    class GameObject: public Object
    {
    public:
        FV_DLL GameObject();

        FV_MO FV_DLL Component* addComponent(u32 type);
        FV_MO FV_DLL bool hasComponent(u32 type);
        FV_MO FV_DLL Component* getComponent(u32 type);
        FV_MO FV_DLL bool removeComponent(u32 type);

        template <class T>
        FV_MO T* addComponent();

        template <class T>
        FV_MO T* getComponent();

        template <class T>
        FV_MO bool hasComponent();

        template <class T>
        FV_MO bool removeComponent();

        FV_MO FV_DLL u32 numComponents();

    private:
        u32 m_Id = -1;
        Map<u32, Component*> m_Components;
    };


    template <class T>
    T* GameObject::addComponent()
    {
        FV_CHECK_MO();
        auto cIt = m_Components.find(T::type());
        Component* pComponent;
        if ( cIt == m_Components.end() )
        {
            pComponent = componentManager()->newComponent<T>();
            pComponent->m_GameObject = this;
            m_Components[T::type()] = pComponent;
        }
        else pComponent = cIt->second;
        return sc<T*>(pComponent);
    }

    template <class T>
    T* GameObject::getComponent()
    {
        FV_CHECK_MO();
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
        FV_CHECK_MO();
        return m_Components.count(T::type()) != 0;
    }

    template <class T>
    bool GameObject::removeComponent()
    {
        FV_CHECK_MO();
        auto cIt = m_Components.find(T::type());
        if ( cIt != m_Components.end() )
        {
            componentManager()->freeComponent<T>(cIt->second);
            m_Components.erase(cIt);
            return true;
        }
        return false;
    }


    FV_DLL ObjectManager<GameObject>* gameObjectManager();
    FV_DLL void deleteGameObjectManager();

}

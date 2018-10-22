#pragma once
#include "GameComponent.h"
#include "../Core/ComponentManager.h"
#include "../Core/Thread.h"

namespace fv
{
    class GameObject: public Object
    {
    public:
        FV_DLL GameObject();

        FV_MO FV_DLL GameComponent* addComponent(u32 type);
        FV_MO FV_DLL bool hasComponent(u32 type);
        FV_MO FV_DLL GameComponent* getComponent(u32 type);

        // Ensure that any ptr to the component is nulled or keep a Ref<ComponentType> handle.
        FV_MO FV_DLL bool removeComponent(u32 type);

        template <class T>
        FV_MO T* addComponent();

        template <class T>
        FV_MO T* getComponent();

        template <class T>
        FV_MO bool hasComponent();

        // Ensure that any ptr to the component is nulled or keep a Ref<ComponentType> handle.
        template <class T>
        FV_MO bool removeComponent();

        FV_MO FV_DLL u32 numComponents();

    private:
        u32 m_Id = -1;
        Map<u32, GameComponent*> m_Components;
    };


    template <class T>
    T* GameObject::addComponent()
    {
        return sc<T*>(addComponent(T::type()));
    }

    template <class T>
    T* GameObject::getComponent()
    {
        return sc<T*>(getComponent(T::type()));
    }

    template <class T>
    bool GameObject::hasComponent()
    {
        return hasComponent(T::type());
    }

    template <class T>
    bool GameObject::removeComponent()
    {
        return removeComponent(T::type());
    }


    FV_DLL SparseArray<GameObject>* gameObjectManager();
    FV_DLL void deleteGameObjectManager();

    FV_DLL GameObject* NewGameObject(bool addSceneComponent=true);
}

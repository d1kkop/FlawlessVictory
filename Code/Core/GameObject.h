#pragma once
#include "ComponentManager.h"
#include "SparseArray.h"
#include "Thread.h"

namespace fv
{
    class Component;
    class TextSerializer;

    class GameObject: public Object
    {
    public:
        FV_MO FV_DLL GameObject();
        FV_MO FV_DLL ~GameObject();

        FV_MO FV_DLL Component* addComponent(u32 type);
        FV_MO FV_DLL bool hasComponent(u32 type);
        FV_MO FV_DLL Component* getComponent(u32 type);

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

        // Ensure that ptrs to components are nulled or keep Ref<ComponentType> handles.
        FV_MO void removeAllComponents();

        FV_MO FV_DLL u32 numComponents();
        FV_MO FV_DLL u32 prototypeId() const;
        FV_MO FV_DLL const String& name() const;
        FV_MO FV_DLL u32 id() const;

        FV_MO FV_DLL void serialize(TextSerializer& ts);

        FV_DLL Mat4& localToWorld() { return *m_LocalToWorld; }
        FV_DLL Mat4& worldToLocal() { return *m_WorldToLocal; }
        FV_DLL Vec3 position() const { return (Vec3)m_LocalToWorld->translation(); }

    private:
        u32 m_PrototypeId = -1;
        String m_Name;
        Map<u32, Component*> m_Components;
        Mat4* m_LocalToWorld{};
        Mat4* m_WorldToLocal{};
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

    // Ensure that every ptr to game object is nulled or keep a Ref<GameObject> handle.
    FV_DLL void DestroyGameObject( GameObject* go );
}

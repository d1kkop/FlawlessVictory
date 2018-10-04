#pragma once
#include "Common.h"
#include "Component.h"

namespace fv
{
    class ComponentManager
    {
        const u32 ComponentBufferSize = 128;

    public:
        ComponentManager() = default;
        ~ComponentManager();

        FV_MO FV_DLL Component* newComponent(u32 type);
        FV_MO FV_DLL void growComponents(u32 type);
        FV_MO FV_DLL Map<u32, Set<Component*>>& components();
        FV_MO FV_DLL void freeComponent(Component* c);
        FV_MO FV_DLL void freeAllOfType(u32 type);
        FV_MO FV_DLL u32 numComponents() const;
        FV_MO FV_DLL u32 numComponents(u32 type);

        FV_MO template <class T> T* newComponent();
        FV_MO template <class T> void freeAllOfType();
        FV_MO template <class T> u32 numComponents();

    private:
        Map<u32, Set<Component*>> m_Components;
        Map<u32, Set<Component*>> m_FreeComponents;
        u32 m_NumComponents = 0;
    };


    template <class T>
    T* ComponentManager::newComponent()
    {
        T* t = sc<T*>(newComponent(T::type()));
        return t;
    }

    template <class T>
    void ComponentManager::freeAllOfType()
    {
        freeAllOfType(T::type());
    }

    template <class T>
    u32 ComponentManager::numComponents()
    {
        return numComponents(T::type());
    }

    FV_MO FV_DLL ComponentManager* componentManager();
    FV_MO FV_DLL void deleteComponentManager();

 //   ComponentIter<T> Itr()
    template <class T>
    Set<T*> Itr()
    {
        auto& components = componentManager()->components()[T::type()];
        return *(Set<T*>*)( &components );
       // return ComponentIter<T>(components);
    }
}

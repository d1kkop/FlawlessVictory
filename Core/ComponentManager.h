#pragma once
#include "Common.h"
#include "Component.h"

namespace fv
{
    class ComponentManager
    {
        const u32 ComponentBufferSize = 32;

    public:
        ComponentManager() = default;
        ~ComponentManager();

        template <class T> T* newComponent();
        template <class T> void freeComponent(Component* comp);
        template <class T> void growComponents();

    private:
        Map<u32, Array<Component*>> m_ActiveComponents;
        Map<u32, Array<Component*>> m_FreeComponents;
    };


    template <class T>
    T* ComponentManager::newComponent()
    {
        growComponents<T>();
        auto& comps  = m_FreeComponents[T::type()];
        Component* c = comps.back();
        comps.pop_back();
        return sc<T*>(c);
    }

    template <class T>
    void ComponentManager::freeComponent(Component* comp)
    {
        m_FreeComponents[T::type()].emplace_back(comp);
    }

    template <class T>
    void ComponentManager::growComponents()
    {
        auto& comps = m_FreeComponents[T::type()];
        if ( comps.size() ) return;
        T* newComps = new T[ComponentBufferSize];
        comps.reserve(ComponentBufferSize);
        for ( u32 i=0; i<ComponentBufferSize; ++i )
        {
            comps.emplace_back(newComps + i);
        }
    }
}

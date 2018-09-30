#pragma once
#include "Common.h"
#include "Component.h"

namespace fv
{
    /*  The ComponentManager serves two purposes.
        1. All components are supposed to exist at all time. Freed components are recycled.
        2. Operations can be performed on a sequential array of memory (roughly) in a ordered fashion. This is very cache friendly. */
    class ComponentManager
    {
        const u32 ComponentBufferSize = 32;

    public:
        ComponentManager() = default;
        ~ComponentManager();

        template <class T> T* newComponent();
        template <class T> void freeComponent(Component* comp);
        template <class T> void growComponents();
        Map<u32, Array<Component*>>& activeComponents() { return m_ActiveComponents; }
        bool executingSingleThreaded() const { return m_ExecutingSingleThreaded; }
        void setExecutingSingleThreaded(bool executingSt) { m_ExecutingSingleThreaded = executingSt; }

    private:
        bool m_ExecutingSingleThreaded = false;
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

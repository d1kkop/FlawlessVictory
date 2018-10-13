#pragma once
#include "PCH.h"
#include "Common.h"
#include "Component.h"

namespace fv
{
    class ComponentManager
    {
    public:
        FV_DLL ComponentManager(u32 componentBufferSize=128);
        FV_DLL ~ComponentManager();

        FV_MO FV_DLL Component* newComponent(u32 type);
        FV_MO FV_DLL void growComponents(u32 type);
        FV_MO FV_DLL Map<u32, Vector<ComponentArray>>& components();
        FV_MO FV_DLL Vector<ComponentArray>& componentsOfType(u32 type);
        FV_MO FV_DLL void freeComponent(Component* c); // Dangerous function. All raw ptrs to these types are not nulled! Refs are!
        FV_MO FV_DLL void freeAllOfType(u32 type); // Dangerous function. All raw ptrs to these types are not nulled! Refs are!
        FV_MO FV_DLL u32 numComponents() const;
        FV_MO FV_DLL u32 numComponents(u32 type);

        template <class T> FV_MO T* newComponent();
        template <class T> FV_MO void freeAllOfType();
        template <class T> FV_MO u32 numComponents();
        template <class T> FV_MO Vector<ComponentArray>& componentsOfType();

    private:
        Map<u32, Vector<ComponentArray>> m_Components;
        Map<u32, Set<Component*>> m_FreeComponents;
        u32 m_NumComponents = 0;
        u32 m_ComponentBufferSize = 128;
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

    template <class T>
    Vector<ComponentArray>& ComponentManager::componentsOfType()
    {
        return componentsOfType(T::type());
    }

    FV_MO FV_DLL ComponentManager* componentManager();
    FV_MO FV_DLL void deleteComponentManager();

    template <class T>
    ComponentCollection<T> Itr()
    {
        Vector<ComponentArray>& components = componentManager()->components()[T::type()];
        return ComponentCollection<T>(components);
    }

    template <class T, class C, class CB>
    void ComponentFor(const C& collection, const CB& cb)
    {
        for ( auto& compArray : collection )
        {
            for ( u32 i=0; i<compArray.size; ++i )
            {
                T* comp = (T*) ((char*)compArray.elements + i*compArray.compSize);
                if ( comp->inUse() ) cb( *comp );
            }
        }
    }
}
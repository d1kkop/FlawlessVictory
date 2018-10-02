#pragma once
#include "../Core/Common.h"
#include "Component.h"

namespace fv
{
    struct ComponentArray
    {
        Component* elements;
        u32 size;
        u32 compSize;
    };


    template <class T>
    struct ComponentIter
    {
        ComponentIter(Vector<ComponentArray>& components):
            m_Components(components) { }

        template <class T>
        struct iterator
        {
            ComponentIter<T>* ptr;

            iterator (ComponentIter<T>* p = nullptr): 
                ptr(p)
            {
                if ( ptr && ptr->m_Components.size() ) m_ElemArray = &ptr->m_Components[0];
                else ptr = nullptr;
            }
            T& operator*() const { return *(((T*)m_ElemArray->elements) + m_Cur); }
            iterator operator++ (int) { iterator tmp = *this; ++*this; return tmp; }
            bool operator== (const iterator& other) const { return ptr == other.ptr; }
            bool operator!= (const iterator& other) const { return ptr != other.ptr; }

            void operator++ () 
            {
                while ( true )
                {
                    if ( ++m_Cur == m_ElemArray->size )
                    {
                        if ( ++m_VecCur == (u32)ptr->m_Components.size() ) { ptr = nullptr; return; } 
                        else { m_Cur = 0; m_ElemArray = &ptr->m_Components[m_VecCur]; }
                    }
                    if ( (((T*)m_ElemArray->elements) + m_Cur)->m_Active ) return;
                }
            }

            const ComponentArray* m_ElemArray = nullptr;
            u32 m_Cur = 0;
            u32 m_VecCur = 0;
        };

        iterator<T> begin() { return iterator<T>(this); }
        iterator<T>& end() { static iterator<T> et; return et; }

    private:
        const Vector<ComponentArray>& m_Components;
    };


    class ComponentManager
    {
        const u32 ComponentBufferSize = 128;

    public:
        ComponentManager() = default;
        ~ComponentManager();

        FV_ST FV_DLL Component* newComponent(u32 type);
        FV_ST FV_DLL void growComponents(u32 type);
        FV_ST FV_DLL Map<u32, Vector<ComponentArray>>& components();
        FV_ST FV_DLL void freeComponent(Component* c);
        FV_ST FV_DLL void freeAllOfType(u32 type);
        FV_ST FV_DLL u32 numComponents() const;
        FV_ST FV_DLL u32 numComponents(u32 type);

        FV_ST template <class T> T* newComponent();
        FV_ST template <class T> void freeAllOfType();
        FV_ST template <class T> u32 numComponents();

    private:
        Map<u32, Vector<ComponentArray>> m_Components;
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

    FV_ST FV_DLL ComponentManager* componentManager();
    FV_ST FV_DLL void deleteComponentManager();

    template <class T>
    ComponentIter<T> Itr()
    {
        Vector<ComponentArray>& components = componentManager()->components()[T::type()];
        return ComponentIter<T>(components);
    }
}

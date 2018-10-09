#pragma once
#include "PCH.h"
#include "Common.h"
#include "Component.h"

namespace fv
{
    struct ComponentArray2
    {
        M<Component>* elements;
        u32 size;
        u32 compSize;
    };

    // Same as componentManager but works with shared ptrs. Is roughly twice as slow when iterating over components.
    class ComponentManager2
    {
    public:
        static const u32 ComponentBufferSize = 128;

    public:
        ComponentManager2() = default;
        ~ComponentManager2();

        FV_MO FV_DLL M<Component> newComponent(u32 type);
        FV_MO FV_DLL void growComponents(u32 type);
        FV_MO FV_DLL Map<u32, Vector<ComponentArray2>>& components();
        FV_MO FV_DLL u32 numComponents() const;
        FV_MO FV_DLL u32 numComponents(u32 type);
        FV_MO FV_DLL void freeComponent(const M<Component>& comp);
        FV_MO FV_DLL void freeAllOfType(u32 type);

        FV_MO template <class T> M<T> newComponent();
        FV_MO template <class T> u32 numComponents();
        FV_MO template <class T> void freeAllOfType();

    private:
        Map<u32, Vector<ComponentArray2>> m_Components;
        Map<u32, Set<M<Component>>> m_FreeComponents;
        u32 m_NumComponents = 0;
    };


    template <class T>
    M<T> ComponentManager2::newComponent()
    {
        return std::static_pointer_cast<T>( newComponent(T::type()) );
    }

    template <class T>
    u32 ComponentManager2::numComponents()
    {
        return numComponents(T::type());
    }

    template <class T>
    void ComponentManager2::freeAllOfType()
    {
        freeAllOfType(T::type());
    }


    template <class T>
    struct ComponentCollection2
    {
        ComponentCollection2(Vector<ComponentArray2>& components):
            m_Components(components) { }

        template <class T>
        struct iterator
        {
            T& operator*() const { return sc<T&>(*m_ElemArray[m_Cur]); }
            iterator operator++ (int) { iterator tmp = *this; ++*this; return tmp; }
            bool operator== (const iterator& other) const { return m_Components == other.m_Components; }
            bool operator!= (const iterator& other) const { return m_Components != other.m_Components; }

            void operator++ ()
            {
                do
                {
                    if ( ++m_Cur == ComponentManager2::ComponentBufferSize )
                    {
                        if ( ++m_VecCur == (u32)m_Components->size() ) { m_Components=nullptr; return; }
                        else { m_Cur = 0; m_ElemArray = (*m_Components)[m_VecCur].elements; }
                    }
                }
                while ( !(m_ElemArray[m_Cur])->inUse() );
            }

            const Vector<ComponentArray2>* m_Components;
            M<Component>* m_ElemArray;
            u32 m_Cur = 0;
            u32 m_VecCur = 0;
        };

        iterator<T> begin() { return iterator<T>{ m_Components.size()?&m_Components:nullptr, m_Components.size()?m_Components[0].elements:nullptr }; }
        iterator<T>& end() { static iterator<T> et {}; return et; }

    private:
        const Vector<ComponentArray2>& m_Components;
    };


    FV_MO FV_DLL ComponentManager2* componentManager2();
    FV_MO FV_DLL void deleteComponentManager2();

    template <class T>
    ComponentCollection2<T> Itr2()
    {
        Vector<ComponentArray2>& components = componentManager2()->components()[T::type()];
        return ComponentCollection2<T>(components);
    }
}

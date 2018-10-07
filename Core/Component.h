#pragma once
#include "PCH.h"
#include "Common.h"
#include "Object.h"

namespace fv
{
    class Component: public Object
    {
    public:
        virtual ~Component() = default;
    };

    struct ComponentArray
    {
        Component* elements;
        u32 size;
        u32 compSize;
    };


    template <class T>
    struct ComponentCollection
    {
        ComponentCollection(Vector<ComponentArray>& components):
            m_Components(components) { }

        template <class T>
        struct iterator
        {
            T& operator*() const { return *(sc<T*>(m_ElemArray) + m_Cur); }
            iterator operator++ (int) { iterator tmp = *this; ++*this; return tmp; }
            bool operator== (const iterator& other) const { return m_Components == other.m_Components; }
            bool operator!= (const iterator& other) const { return m_Components != other.m_Components; }

            void operator++ ()
            {
                do
                {
                    if ( ++m_Cur == ComponentManager::ComponentBufferSize )
                    {
                        if ( ++m_VecCur == (u32)m_Components->size() ) { m_Components=nullptr; return; }
                        else { m_Cur = 0; m_ElemArray = (*m_Components)[m_VecCur].elements; }
                    }
                } while (!(sc<T*>(m_ElemArray) + m_Cur)->inUse());
            }

            const Vector<ComponentArray>* m_Components;
            Component* m_ElemArray;
            u32 m_Cur = 0;
            u32 m_VecCur = 0;
        };

        iterator<T> begin() { return iterator<T>{ m_Components.size()?&m_Components:nullptr, m_Components.size()?m_Components[0].elements:nullptr }; }
        iterator<T>& end() { static iterator<T> et{}; return et; }

    private:
        const Vector<ComponentArray>& m_Components;
    };
}
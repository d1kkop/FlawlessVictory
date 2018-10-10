#pragma once
#include "PCH.h"
#include "Common.h"
#include "Object.h"
#include "Reflection.h"

namespace fv
{
    class Component: public Object, public Type
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
                    if ( ++m_Cur == m_ArrayLength )
                    {
                        if ( ++m_VecCur == (u32)m_Components->size() ) { m_Components=nullptr; return; }
                        else { m_Cur = 0; m_ElemArray = (*m_Components)[m_VecCur].elements; }
                    }
                } while (!(sc<T*>(m_ElemArray) + m_Cur)->inUse());
            }

            const Vector<ComponentArray>* m_Components;
            Component* m_ElemArray;
            u32 m_ArrayLength;
            u32 m_Cur = 0;
            u32 m_VecCur = 0;
        };

        iterator<T> begin() 
        { 
            bool b = m_Components.size()!=0;
            return iterator<T>
            { 
                b?&m_Components:nullptr, 
                b?m_Components[0].elements:nullptr,
                b?m_Components[0].size:0
            };
        }
        iterator<T>& end() { static iterator<T> et{}; return et; }

    private:
        const Vector<ComponentArray>& m_Components;
    };
}
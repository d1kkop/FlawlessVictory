#pragma once
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
                    if ( (((T*)m_ElemArray->elements) + m_Cur)->inUse() ) return;
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
}
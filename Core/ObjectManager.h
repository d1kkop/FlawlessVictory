#pragma once
#include "Common.h"
#include "Thread.h"
#include "Algorithm.h"

namespace fv
{
    class Object
    {
    public:
        bool m_Freed = false;
        bool m_Active = false;
    };

    struct ObjectArray
    {
        Object* elements;
        u32 size;
    };

    template <class T>
    class ObjectManager
    {
    public:
        ObjectManager(u32 objBufferSize=128);
        ~ObjectManager();

        FV_MO T* newObject();
        FV_MO void freeObject(T* object);
        FV_MO u32 numObjects() const;

    private:
        void growObjects();
        u32 m_ObjectBufferSize;
        Vector<ObjectArray> m_Objects;
        Set<T*> m_FreeObjects;
        u32 m_NumObjects = 0;
    };


    template <class T>
    ObjectManager<T>::ObjectManager(u32 objBufferSize):
        m_ObjectBufferSize(objBufferSize)
    {
    }

    template <class T>
    ObjectManager<T>::~ObjectManager()
    {
        for ( auto& objArray : m_Objects )
            delete [] objArray.elements;
    }

    template <class T>
    T* ObjectManager<T>::newObject()
    {
        FV_CHECK_MO();
        if ( m_FreeObjects.empty() )
        {
            T* objs = new T[m_ObjectBufferSize];
            for ( u32 i=0; i<m_ObjectBufferSize; ++i )
            {
                m_FreeObjects.insert( objs + i );
            }
            ObjectArray objArray = { objs, m_ObjectBufferSize };
            m_Objects.emplace_back( objArray );
        }
        T* o = *m_FreeObjects.begin();
        m_FreeObjects.erase(m_FreeObjects.begin());
        if ( o->m_Freed ) new (o)T; // Only call if object was recycled.
        o->m_Active = true;
        m_NumObjects++;
        return o;
    }

    template <class T>
    void ObjectManager<T>::freeObject(T* object)
    {
        FV_CHECK_MO();
        assert( !object->m_Freed && object->m_Active );
        assert( m_FreeObjects.count(object) == 0 );
        object->m_Freed  = true; // Do not remove from objectList to avoid fragmentation.
        object->m_Active = false;
        m_FreeObjects.insert(object);
        m_NumObjects--;
    }

    template <class T>
    u32 ObjectManager<T>::numObjects() const
    {
        return m_NumObjects;
    }

}
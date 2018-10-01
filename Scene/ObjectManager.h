#pragma once
#include "../Core/Common.h"
#include "../Core/Thread.h"
#include "../Core/Algorithm.h"

namespace fv
{
    /*  The ObjectManager serves two purposes.
        1. All objects are supposed to exist at all time. Freed objects are recycled.
        2. Operations can be performed on a sequential array of memory (roughly) in a ordered fashion. This is very cache friendly. */
    template <class T>
    class ObjectManager
    {
    public:
        ObjectManager(u32 objBufferSize=128);
        ~ObjectManager();

        FV_ST T* newObject();
        FV_ST void freeObject(T* object);

    private:
        void growObjects();
        u32 m_ObjectBufferSize;
        Set<T*> m_ActiveObjects;
        Array<T*> m_FreeObjects;
    };


    template <class T>
    ObjectManager<T>::ObjectManager(u32 objBufferSize):
        m_ObjectBufferSize(objBufferSize)
    {
    }

    template <class T>
    ObjectManager<T>::~ObjectManager()
    {
        for ( auto* o : m_ActiveObjects ) 
        for ( auto* o : m_FreeObjects ) delete o;
    }

    template <class T>
    T* ObjectManager<T>::newObject()
    {
        FV_CHECK_ST();
        growObjects();
        T* o = m_FreeObjects.back();
        m_FreeObjects.pop_back();
        m_ActiveObjects.insert(o);
        return o;
    }

    template <class T>
    void ObjectManager<T>::freeObject(T* object)
    {
        FV_CHECK_ST();
        assert( !Contains(m_FreeObjects, object) );
        m_FreeObjects.push_back( object );
        m_ActiveObjects.erase(object);
    }

    template <class T>
    void ObjectManager<T>::growObjects()
    {
        FV_CHECK_ST();
        if ( m_FreeObjects.size() ) return;
        T* objs = new T[m_ObjectBufferSize];
        m_FreeObjects.reserve(m_ObjectBufferSize);
        for ( u32 i=0; i<m_ObjectBufferSize; ++i )
        {
            m_FreeObjects.emplace_back(objs + i);
        }
    }
}
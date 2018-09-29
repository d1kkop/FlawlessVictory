#pragma once
#include "Platform.h"
#include "Common.h"


namespace fv
{
    template <class T>
    class ObjectManager
    {
    public:
        ObjectManager(u32 objBufferSize=128):
            m_ObjectBufferSize(objBufferSize) { }
        ~ObjectManager();

        T* newObject();

    private:
        void growObjects();
        u32 m_ObjectBufferSize;
        Set<T*> m_ActiveObjects;
        Array<T*> m_FreeObjects;
    };


    template <class T>
    ObjectManager<T>::~ObjectManager()
    {
        for ( auto* o : m_ActiveObjects ) 
        for ( auto* o : m_FreeObjects ) delete o;
    }

    template <class T>
    T* ObjectManager<T>::newObject()
    {
        growObjects();
        T* o = m_FreeObjects.back();
        m_FreeObjects.pop_back();
        m_ActiveObjects.insert(o);
        return o;
    }

    template <class T>
    void ObjectManager<T>::growObjects()
    {
        if ( m_FreeObjects.size() ) return;
        T* objs = new T[m_ObjectBufferSize];
        m_FreeObjects.reserve(m_ObjectBufferSize);
        for ( u32 i=0; i<m_ObjectBufferSize; ++i )
        {
            m_FreeObjects.emplace_back(objs + i);
        }
    }
}
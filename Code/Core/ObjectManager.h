#pragma once
#include "Object.h"
#include "Thread.h"

namespace fv
{
    template <class T>
    class ObjectManager
    {
    public:
        ObjectManager(u32 objBufferSize=128, bool threadSafe=false);
        ~ObjectManager();

        T* newObject();
        void freeObject(T* object);

    private:
        void growObjects();
        u32 m_ObjectBufferSize;
        Vector<ObjectArray> m_Objects;
        Set<T*> m_FreeObjects;
        u32 m_NumObjects = 0;
        bool m_ThreadSafe = false;
        Mutex m_ObjectsMutex;
    };


    template <class T>
    ObjectManager<T>::ObjectManager(u32 objBufferSize, bool threadSafe):
        m_ObjectBufferSize(objBufferSize),
        m_ThreadSafe(threadSafe)
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
        if ( m_ThreadSafe )
        {
            m_ObjectsMutex.lock();
        }
    #if FV_DEBUG
        else
        {
             FV_CHECK_MO(); 
        }
    #endif
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
        if ( m_ThreadSafe )
        {
            m_ObjectsMutex.unlock();
        }
        u32 oldVersion = o->m_Version;
        if ( o->m_Freed ) new (o)T; // Only call if object was recycled.
        o->m_Active = true;
        o->m_Version = oldVersion+1;
        m_NumObjects++;
        return o;
    }

    template <class T>
    void ObjectManager<T>::freeObject(T* object)
    {
        if (!object) return;
        if ( !object->m_Freed && object->m_Active ) // Allow multiple calls to freeObject
        {
            // assert( !object->m_Freed && object->m_Active );
            object->m_Freed  = true; // Do not remove from objectList to avoid fragmentation.
            object->m_Active = false;
            if ( m_ThreadSafe )
            {
                m_ObjectsMutex.lock();
            }
        #if FV_DEBUG
            else
            {
                FV_CHECK_MO();
            }
        #endif
            assert( m_FreeObjects.count(object) == 0 );
            m_FreeObjects.insert(object);
            m_NumObjects--;
            if ( m_ThreadSafe )
            {
                m_ObjectsMutex.unlock();
            }
        }
    }

}
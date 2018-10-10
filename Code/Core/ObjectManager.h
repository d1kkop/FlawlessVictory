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
        void freeAll();

    private:
        void growObjects();
        u32 m_ObjectBufferSize;
        Vector<ObjectArray> m_ObjectArrays;
        Set<T*> m_FreeObjects;
        u32 m_NumObjects = 0;
        bool m_ThreadSafe = false;
        RMutex m_ObjectsMutex;
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
        for ( auto& objArray : m_ObjectArrays )
            if ( m_ObjectBufferSize != 1 )
                delete [] (T*)objArray.elements;
            else
                delete (T*)objArray.elements;
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
            m_ObjectArrays.emplace_back( objArray );
        }
        T* o = *m_FreeObjects.begin();
        m_FreeObjects.erase(m_FreeObjects.begin());
        m_NumObjects++;
        if ( m_ThreadSafe )
        {
            m_ObjectsMutex.unlock();
        }
        o->m_Freed = false;
        return o;
    }

    template <class T>
    void ObjectManager<T>::freeObject(T* o)
    {
        if ( !o->m_Freed )
        {
            u32 oldVersion = o->m_Version;
            new (o)T; // Resets object (placement new)
            o->m_Version = oldVersion+1;  // Increment version immediately so tat refs to this component will return null ptr from now on.
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
            assert( m_FreeObjects.count(o) == 0 );
            m_FreeObjects.insert(o);
            m_NumObjects--;
            if ( m_ThreadSafe )
            {
                m_ObjectsMutex.unlock();
            }
        }
        else
        {
            LOGW("Tried to free an object that was already freed. Call ignored.");
        }
    }

    template <class T>
    void ObjectManager<T>::freeAll()
    {
        if ( m_ThreadSafe )
        {
            m_ObjectsMutex.lock();
        }
        else
        {
            FV_CHECK_MO();
        }
        for ( auto& objArray : m_ObjectArrays )
        {
            for ( u32 i=0; i<objArray.size; ++i )
            {
                T* obj = (T*)objArray.elements + i*sizeof(T);
                if ( !obj->m_Freed ) freeObject( obj );
            }
        }
        if ( m_ThreadSafe )
        {
            m_ObjectsMutex.unlock();
        }
    }
}
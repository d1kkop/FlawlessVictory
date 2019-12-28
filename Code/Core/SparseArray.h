#pragma once
#include "Object.h"
#include "Thread.h"

namespace fv
{
    template <class T>
    class SparseArray
    {
    public:
        SparseArray(u32 objBufferSize=128, bool threadSafe=false);
        ~SparseArray();
        void purge();

        T* newObject();
        void freeObject(T* object);
        const Vector<ObjectArray>& allElements() const { return m_ObjectArrays; }

    private:
        u32 m_ObjectBufferSize;
        Vector<ObjectArray> m_ObjectArrays;
        Set<T*> m_FreeObjects;
        u32 m_NumObjects = 0;
        bool m_ThreadSafe = false;
        RMutex m_ObjectsMutex;
    };


    template <class T>
    SparseArray<T>::SparseArray(u32 objBufferSize, bool threadSafe):
        m_ObjectBufferSize(objBufferSize),
        m_ThreadSafe(threadSafe)
    {
    }

    template <class T>
    SparseArray<T>::~SparseArray()
    {
        purge();
    }

    template <class T>
    void fv::SparseArray<T>::purge()
    {
        for ( auto& objArray : m_ObjectArrays )
        {
            if ( m_ObjectBufferSize != 1 )
                delete [] (T*)objArray.elements;
            else
                delete (T*)objArray.elements;
        }
        m_ObjectArrays.clear();
    }

    template <class T>
    T* SparseArray<T>::newObject()
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
        u32 oldVersion = o->m_Version;
        new (o)T; // Resets object (placement new)
        o->m_Freed = false;
        o->m_Version = oldVersion;
        return o;
    }

    template <class T>
    void SparseArray<T>::freeObject(T* o)
    {
        if ( !o->m_Freed )
        {
            o->~T(); // Call destructor
            o->m_Version++; // Refs will return nullptr from now on
            o->m_Freed = true;
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
}
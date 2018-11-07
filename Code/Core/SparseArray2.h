#pragma once
#include "Common.h"

namespace fv
{
    template <class T>
    class SparseArray2
    {
    public:
        SparseArray2(u32 arraySize=128, bool threadSafe=false):
            m_ArraySize(arraySize), m_ThreadSafe(threadSafe)
        {
            assert(arraySize>0);
        }
        ~SparseArray2() { purge(); }

        T* newObject();
        void freeObject(T* obj);
        void purge();

    private:
        Vector<Vector<T>*> m_Arrays;
        Set<T*> m_FreeObjects;
        u32 m_ArraySize;
        bool m_ThreadSafe;
    };


    template <class T>
    T* SparseArray2<T>::newObject()
    {
        if ( m_FreeObjects.empty() )
        {
            m_Arrays.emplace_back( new Vector<T>( m_ArraySize ) );
            for ( auto& obj : *m_Arrays.back() )
            {
                m_FreeObjects.insert( &obj );
            }
        }
        T* freeObj = *m_FreeObjects.begin();
        m_FreeObjects.erase( m_FreeObjects.begin() );
        return freeObj;
    }

    template <class T>
    void SparseArray2<T>::freeObject(T* obj)
    {
        if ( !obj ) return;
        m_FreeObjects.insert( obj );
    }

    template <class T>
    void SparseArray2<T>::purge()
    {
        for ( auto* ar : m_Arrays ) delete ar;
        m_FreeObjects.clear();
        m_Arrays.clear();
    }
}
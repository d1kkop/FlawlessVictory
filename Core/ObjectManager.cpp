#include "ObjectManager.h"
#include "Object.h"


namespace fv
{
    ObjectManager::ObjectManager()
    {
    }

    Object* ObjectManager::newObject()
    {
        growObjects();
        Object* e = m_FreeObjects.back();
        m_FreeObjects.pop_back();
        m_ActiveObjects.insert( e );
        return e;
    }

    void ObjectManager::growObjects()
    {
        if ( m_FreeObjects.size() ) return;
        Object* ents = new Object[ObjectBufferSize];
        m_FreeObjects.reserve( ObjectBufferSize );
        for ( u32 i=0; i<ObjectBufferSize; ++i )
        {
            m_FreeObjects.emplace_back(ents + i);
        }
    }
}
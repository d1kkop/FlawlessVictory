#include "ComponentManager.h"
#include "../Core/Algorithm.h"
#include "../Core/Functions.h"
#include "../Core/Reflection.h"
#include "../Core/Thread.h"
#include <cassert>

namespace fv
{
    ComponentManager::~ComponentManager()
    {
        for ( auto& kvp : m_Components ) 
        {
            Vector<ComponentArray>& comps = kvp.second;
            for ( auto& ar : comps )
                delete [] ar.elements;
        }
    }

    Component* ComponentManager::newComponent(u32 type)
    {
        FV_CHECK_MO();
        const TypeInfo& ti = typeManager()->typeInfo(type);
        assert( ti.hash == type );
        auto& freeComps = m_FreeComponents[type];
        if ( freeComps.empty() )
        {
            auto& freeComps = m_FreeComponents[type];
            Type* newComps = typeManager()->createType( type, ComponentBufferSize ); // Allocates contiguous array
            assert( freeComps.size() == 0 );
            for ( u32 i=0; i<ComponentBufferSize; ++i )
            {
                Component* c = (Component*)((char*)newComps + i*ti.size);
                freeComps.insert( c );
            }
            auto& comps = m_Components[type];
            ComponentArray ca = { (Component*)newComps, ComponentBufferSize, ti.size };
            comps.emplace_back( ca ); 
        }
        Component* c = *freeComps.begin();
        freeComps.erase(freeComps.begin());
        if ( c->m_Freed ) 
        {
            ti.resetFunc( c ); // Only call placement new when object is recycled.
        }
        c->m_Active = true;
        m_NumComponents++;
        return c;
    }

    void ComponentManager::freeComponent(Component* c)
    {
        FV_CHECK_MO();
        if ( !c->m_Freed && c->m_Active ) // Allow multiple calls to freeComponent
        {
            // assert( !c->m_Freed && c->m_Active );
            auto& freeComps = m_FreeComponents[c->type()];
            assert( freeComps.count(c)==0 );
            c->m_Freed  = true; // NOTE: Do not remove from components list to avoid fragmentation.
            c->m_Active = false;
            freeComps.insert( c );
            m_NumComponents--;
        }
    }

    void ComponentManager::freeAllOfType(u32 type)
    {
        FV_CHECK_MO();
        auto& compVector = m_Components[type];
        for ( auto& compArray : compVector )
        {
            for ( u32 i = 0; i < compArray.size; i++ )
            {
                Component* c = (Component*)((char*)compArray.elements + i*compArray.compSize);
                if ( !c->m_Freed && c->m_Active )
                {
                    freeComponent(c);
                }
            }
        }
    }

    u32 ComponentManager::numComponents() const
    {
        FV_CHECK_MO();
        return m_NumComponents;
    }

    u32 ComponentManager::numComponents(u32 type)
    {
        FV_CHECK_MO();
        const auto& compsVector = m_Components[type];
        u32 k=0;
        for ( const auto& compArray : compsVector )
        {
            k += compArray.size;
        }
        return k;
    }

    Map<u32, Vector<ComponentArray>>& ComponentManager::components()
    {
        FV_CHECK_MO();
        return m_Components;
    }

    ComponentManager* g_ComponentManager {};
    ComponentManager* componentManager() { return CreateOnce(g_ComponentManager); }
    void deleteComponentManager() { delete g_ComponentManager; g_ComponentManager=nullptr; }

}
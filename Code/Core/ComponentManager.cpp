#include "PCH.h"
#include "ComponentManager.h"
#include "Algorithm.h"
#include "Functions.h"
#include "Reflection.h"
#include "Thread.h"
#include <cassert>

namespace fv
{

    ComponentManager::ComponentManager(u32 componentBufferSize):
        m_ComponentBufferSize(componentBufferSize)
    {
    }

    ComponentManager::~ComponentManager()
    {
        for ( auto& kvp : m_Components ) 
        {
            Vector<ComponentArray>& objs = kvp.second;
            for ( auto& ar : objs )
                if ( m_ComponentBufferSize != 1 )
                    delete [] ar.elements;
                else
                    delete ar.elements;
        }
    }

    Component* ComponentManager::newComponent(u32 type)
    {
        FV_CHECK_MO();
        const TypeInfo* ti = typeManager()->typeInfo(type);
        if (!ti) 
        {
            LOGW("Invalid type %d, component NOT created.", type);
            return nullptr;
        }
        assert( ti->hash == type );
        auto& freeComps = m_FreeComponents[type];
        if ( freeComps.empty() )
        {
            Component* newComps = sc<Component*>(typeManager()->createTypes( *ti, m_ComponentBufferSize )); // Allocates contiguous array
            assert( newComps && freeComps.size() == 0 );
            for ( u32 i=0; i<m_ComponentBufferSize; ++i )
            {
                Component* c = (Component*)( (char*)newComps + i*ti->size );
                freeComps.insert( c );
            }
            auto& comps = m_Components[type];
            ComponentArray ca = { newComps, m_ComponentBufferSize, ti->size };
            comps.emplace_back( ca ); 
            // check base types
            if ( ti->flags & FV_UPDATE ) m_UpdateComponents[type].emplace_back( ca );
            if ( ti->flags & FV_PHYSICS ) m_PhysicsComponents[type].emplace_back( ca );
            if ( ti->flags & FV_NETWORK ) m_NetworkComponents[type].emplace_back( ca );
            if ( ti->flags & FV_DRAW ) m_DrawComponents[type].emplace_back( ca );
        }
        Component* c = *freeComps.begin();
        freeComps.erase(freeComps.begin());
        u32 oldVersion = c->m_Version; // Version was already incremented when freed.
        ti->resetFunc( c ); // Calls constructor without with reused memory
        c->m_Freed = false;
        c->m_Version = oldVersion;
        TypeManager::setType( type, *c );
        m_NumComponents++;
        return c;
    }

    void ComponentManager::freeComponent(Component* c)
    {
        FV_CHECK_MO();
        if ( !c->m_Freed )
        {
            const TypeInfo* ti = typeManager()->typeInfo(c->type());
            if ( !ti )
            {
                LOGW("Invalid type %d, component NOT freed.", c->type());
                return;
            }
            assert(ti->hash == c->type());
            auto& freeComps = m_FreeComponents[c->type()];
            assert( freeComps.count(c)==0 );
            c->~Component(); // Call destructor without releasing the memory
            c->m_Version++; // By incrementing the version, the cached references will return nullptr from now on.
            c->m_Freed = true;
            freeComps.insert( c );
            m_NumComponents--;
        }
        else
        {
            LOGW("Tried to free a component that was already freed. Call ignored.");
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
                if (!c->m_Freed) freeComponent(c);
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

    Map<u32, Vector<ComponentArray>>& ComponentManager::updateComponents()
    {
        FV_CHECK_MO();
        return m_UpdateComponents;
    }

    Map<u32, Vector<ComponentArray>>& ComponentManager::physicsComponents()
    {
        FV_CHECK_MO();
        return m_PhysicsComponents;
    }

    Map<u32, Vector<ComponentArray>>& ComponentManager::networkComponents()
    {
        FV_CHECK_MO();
        return m_NetworkComponents;
    }

    Map<u32, Vector<ComponentArray>>& ComponentManager::drawComponents()
    {
        FV_CHECK_MO();
        return m_DrawComponents;
    }

    Vector<ComponentArray>& ComponentManager::componentsOfType(u32 type)
    {
        FV_CHECK_MO();
        return m_Components[type]; // Insert empty array if was not there
    }

    ComponentManager* g_ComponentManager {};
    ComponentManager* componentManager() { return CreateOnce(g_ComponentManager); }
    void deleteComponentManager() { delete g_ComponentManager; g_ComponentManager=nullptr; }

}
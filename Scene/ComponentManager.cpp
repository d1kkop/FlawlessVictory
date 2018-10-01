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
            for ( auto* c : kvp.second ) delete c;
        for ( auto& kvp : m_FreeComponents )
            for ( auto* c : kvp.second ) delete c;
    }

    Component* ComponentManager::newComponent(u32 type)
    {
        FV_CHECK_ST();
        growComponents(type);
        auto& freeComps  = m_FreeComponents[type];
        Component* c = freeComps.back();
        freeComps.pop_back();
        TypeInfo& ti = typeManager()->typeInfo(type);
        ti.resetFunc(c);
        c->m_Type = ti.hash;
        if ( c->updatable() )
            m_UpdatableComponents[type].emplace_back( c );
        else 
            m_Components[type].emplace_back( c );
        return c;
    }

    void ComponentManager::freeComponent(Component* c)
    {
        FV_CHECK_ST();
        if ( c->updatable() )
        {
            assert( Contains(m_UpdatableComponents[c->type()], c) );
            Remove( m_UpdatableComponents[c->type()], c );
        }
        else
        {
            assert( Contains(m_Components[c->type()], c) );
            Remove( m_Components[c->type()], c );
        }
        auto& freeComps = m_FreeComponents[c->type()];
        assert( !Contains(freeComps, c) );
        freeComps.emplace_back( c );
    }

    void ComponentManager::growComponents(u32 type)
    {
        FV_CHECK_ST();
        auto& freeComps = m_FreeComponents[type];
        if ( freeComps.size() ) return;
        TypeInfo& ti = typeManager()->typeInfo(type);
        Component* newComps = ti.createFunc(ComponentBufferSize);
        freeComps.reserve(ComponentBufferSize);
        for ( u32 i=0; i<ComponentBufferSize; ++i )
        {
            freeComps.emplace_back(newComps + i);
        }
    }

    Map<u32, Array<Component*>>& ComponentManager::components()
    {
        FV_CHECK_ST();
        return m_Components;
    }


    Map<u32, Array<Component*>>& ComponentManager::updatableComponents()
    {
        FV_CHECK_ST();
        return m_UpdatableComponents;
    }

    ComponentManager* g_ComponentManager {};
    ComponentManager* componentManager() { return CreateOnce(g_ComponentManager); }
}
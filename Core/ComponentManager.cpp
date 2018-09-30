#include "ComponentManager.h"
#include "Algorithm.h"
#include "Functions.h"
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
        growComponents(type);
        auto& comps  = m_FreeComponents[type];
        Component* c = comps.back();
        comps.pop_back();
        TypeInfo& ti = typeManager()->typeInfo(type);
        ti.resetFunc(c);
        c->m_InFreeList = false;
        c->m_Type = ti.hash;
        return c;
    }

    void ComponentManager::freeComponent(Component* c)
    {
        assert(m_FreeComponents.count(c->type())!=0);
        auto& components = m_FreeComponents[c->type()];
        assert( find_if( components, [c](auto* cl) { return c==cl; }) == components.end() );
        components.emplace_back( c );
    }

    void ComponentManager::growComponents(u32 type)
    {
        assert(m_Components.count(type)!=0);
        auto& comps = m_FreeComponents[type];
        if ( comps.size() ) return;
        TypeInfo& ti = typeManager()->typeInfo(type);
        Component* newComps = ti.createFunc(ComponentBufferSize);
        comps.reserve(ComponentBufferSize);
        for ( u32 i=0; i<ComponentBufferSize; ++i )
        {
            comps.emplace_back(newComps + i);
        }
    }

    Map<u32, Array<Component*>>& ComponentManager::components()
    {
        return m_Components;
    }


    ComponentManager* g_ComponentManager{};
    ComponentManager* componentManager() { return createOnce(g_ComponentManager); }
}
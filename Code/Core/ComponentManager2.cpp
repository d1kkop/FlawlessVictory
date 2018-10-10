#include "PCH.h"
#include "ComponentManager2.h"
#include "Algorithm.h"
#include "Functions.h"
#include "Reflection.h"
#include "Thread.h"
#include <cassert>

namespace fv
{
    ComponentManager2::~ComponentManager2()
    {
           for ( auto& kvp : m_Components )
            {
                Vector<ComponentArray2>& objs = kvp.second;
                for ( auto& ar : objs )
                    if ( ComponentBufferSize != 1 )
                        delete [] ar.elements;
                    else
                        delete ar.elements;
            }
    }

    M<Component> ComponentManager2::newComponent(u32 type)
    {
        FV_CHECK_MO();
        const TypeInfo* ti = typeManager()->typeInfo(type);
        if (!ti) 
        {
            LOGC("Invalid type %d, component NOT created.", type);
            return nullptr;
        }
        assert( ti->hash == type );
        auto& freeComps = m_FreeComponents[type];
        if ( freeComps.empty() )
        {
            assert( freeComps.size() == 0 );
            M<Component>* sharedArray = new M<Component>[ComponentBufferSize];
            for ( u32 i=0; i<ComponentBufferSize; ++i )
            {
                Type* newComp = typeManager()->createTypes( *ti, 1 );
                sharedArray[i] = M<Component>( sc<Component*>(newComp) );
                freeComps.insert( sharedArray[i] );
            }
            auto& comps = m_Components[type];
            ComponentArray2 ca = { sharedArray, ComponentBufferSize, ti->size };
            comps.emplace_back( ca ); 
        }
        M<Component> c = *freeComps.begin();
        freeComps.erase(freeComps.begin());
        c->m_Freed = false;
        TypeManager::setType( type, *c );
        m_NumComponents++;
        return c;
    }

    void ComponentManager2::freeComponent(const M<Component>& c)
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
            assert(freeComps.count(c)==0);
            u32 oldVersion = c->m_Version;
            ti->resetFunc(c.get()); // Resets object (placement new)
            c->m_Version = oldVersion+1; // Increment version immediately so tat refs to this component will return null ptr from now on.
            freeComps.insert(c);
            m_NumComponents--;
        }
        else
        {
            LOGW("Tried to free a component that was already freed. Call ignored.");
        }
    }

    void ComponentManager2::freeAllOfType(u32 type)
    {
        FV_CHECK_MO();
        auto& compVector = m_Components[type];
        for ( auto& compArray : compVector )
        {
            for ( u32 i = 0; i < compArray.size; i++ )
            {
                M<Component>& c = compArray.elements[i];
                if ( !c->m_Freed ) freeComponent(c);
            }
        }
    }

    u32 ComponentManager2::numComponents() const
    {
        FV_CHECK_MO();
        return m_NumComponents;
    }

    u32 ComponentManager2::numComponents(u32 type)
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

    Map<u32, Vector<ComponentArray2>>& ComponentManager2::components()
    {
        FV_CHECK_MO();
        return m_Components;
    }

    ComponentManager2* g_ComponentManager2 {};
    ComponentManager2* componentManager2() { return CreateOnce(g_ComponentManager2); }
    void deleteComponentManager2() { delete g_ComponentManager2; g_ComponentManager2=nullptr; }

}
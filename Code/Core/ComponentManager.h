#pragma once
#include "PCH.h"
#include "Common.h"
#include "Component.h"
#include "Algorithm.h"

namespace fv
{
    class ComponentManager
    {
    public:
        FV_DLL ComponentManager( u32 componentBufferSize=64 );
        FV_DLL ~ComponentManager();

        FV_MO FV_DLL M<Component> newComponent( u32 type );
        FV_MO FV_DLL Map<u32, Vector<ComponentArray>>& components();
        FV_MO FV_DLL Map<u32, Vector<ComponentArray>>& updateComponents();
        FV_MO FV_DLL Map<u32, Vector<ComponentArray>>& physicsComponents();
        FV_MO FV_DLL Map<u32, Vector<ComponentArray>>& networkComponents();
        FV_MO FV_DLL Map<u32, Vector<ComponentArray>>& drawComponents();
        FV_MO FV_DLL Vector<ComponentArray>& componentsOfType( u32 type );
        FV_MO FV_DLL u32 numComponents() const;
        FV_MO FV_DLL u32 numComponents( u32 type );

        template <class T> FV_MO M<T> newComponent();
        template <class T> FV_MO u32 numComponents();
        template <class T> FV_MO Vector<ComponentArray>& componentsOfType();

        FV_MO FV_DLL void freeAllMemory();

    private:
        FV_MO static void recycleFromShared( Component *c );
        FV_MO void recycleComponent( Component* c ); // Dangerous function. All raw ptrs to these types are not nulled! Refs are!

        Map<u32, Vector<ComponentArray>> m_Components;
        Map<u32, Vector<ComponentArray>> m_UpdateComponents;
        Map<u32, Vector<ComponentArray>> m_PhysicsComponents;
        Map<u32, Vector<ComponentArray>> m_NetworkComponents;
        Map<u32, Vector<ComponentArray>> m_DrawComponents;
        Map<u32, Set<Component*>> m_FreeComponents; // Use set instead of Unsorted_set as otherwise lower addresses are not selected first (which is better).
        u32 m_NumComponents = 0;
        u32 m_ComponentBufferSize = 128;
    };

    template <class T>
    M<T> ComponentManager::newComponent()
    {
        return spc<T>( newComponent( T::type() ) );
    }

    template <class T>
    u32 ComponentManager::numComponents()
    {
        return numComponents( T::type() );
    }

    template <class T>
    Vector<ComponentArray>& ComponentManager::componentsOfType()
    {
        return componentsOfType( T::type() );
    }

    FV_MO FV_DLL ComponentManager* componentManager();
    FV_MO FV_DLL void deleteComponentManager();

    template <class T>
    ComponentCollection<T> Itr()
    {
        Vector<ComponentArray>& components = componentManager()->components()[T::type()];
        return ComponentCollection<T>( components );
    }

    template <class T, class C, class CB>
    void ComponentFor( const C& collection, const CB& cb )
    {
        for ( auto& compArray : collection )
        {
            for ( u32 i=0; i<compArray.size; ++i )
            {
                T* comp = (T*)((char*)compArray.elements + i*compArray.compSize);
                if ( comp->inUse() ) cb( *comp );
            }
        }
    }

    inline void PrepareList( Map<u32, Vector<ComponentArray>>& components, Vector<ComponentArray>& listOut )
    {
        listOut.clear();
        for ( auto& kvp : components )
        {
            Vector<ComponentArray>& compArrayList = kvp.second;
            for ( auto& compArray : compArrayList )
            {
                if ( compArray.size > 0 )
                {
                    listOut.emplace_back( compArray );
                }
            }
        }
    }

    template <class CmpFunc>
    inline void PrepareList( Map<u32, Vector<ComponentArray>>& components, Vector<ComponentArray>& listOut, const CmpFunc& cmpFunc )
    {
        PrepareList( components, listOut );
        Sort( listOut, cmpFunc );
    }

    template <class T, class Cb>
    inline void Flatten( Map<u32, Vector<ComponentArray>>& components, const Cb& cb )
    {
        for ( auto& kvp : components )
        {
            Vector<ComponentArray>& compArrayList = kvp.second;
            for ( auto& compArray : compArrayList )
            {
                for ( u32 i=0; i<compArray.size; ++i )
                {
                    T* c = (T*)((char*)compArray.elements + i*compArray.compSize);
                    if ( c->inUse() ) cb( c );
                }
            }
        }
    }

    inline void Flatten( Map<u32, Vector<ComponentArray>>& components, Vector<Component*>& listOut )
    {
        listOut.clear();
        Flatten<Component>( components, [&] ( Component* c ) { listOut.emplace_back( c ); } );
    }

    inline void Flatten( Map<u32, Vector<ComponentArray>>& components, Vector<Component*>* listOut, u32 numLists )
    {
        u32 i;
        for ( i=0; i<numLists; ++i ) listOut[i].clear();
        i = 0;
        Flatten<Component>( components, [&] ( Component* c )
        {
            listOut[i++].emplace_back( c );
            if ( i == numLists ) i = 0;
        } );
    }

    template <class CmpFunc>
    inline void Flatten( Map<u32, Vector<ComponentArray>>& components, Vector<Component*>& listOut, const CmpFunc& cmpFunc )
    {
        Flatten( components, listOut );
        Sort( listOut, cmpFunc );
    }

}

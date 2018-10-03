#include "SystemManager.h"
#include "../Core/Algorithm.h"
#include "../Core/Time.h"
#include "../Core/Thread.h"
#include "../Core/OSLayer.h"
#include "../Core/InputManager.h"
#include "../Scene/ComponentManager.h"

namespace fv
{
    bool SystemManager::initialize(const SystemParams& params)
    {
        if ( !OSInitialize() )
        {
            return false;
        }
        if ( !OSLoadLibrary( params.moduleName.c_str() ) )
        {
            return false;
        }
        if ( !(m_Window = OSCreateWindow( params.moduleName.c_str(), 100, 100, params.windowWidth, params.windowHeight, params.fullscreen )) )
        {
            return false;
        }
        return true;
    }

    void SystemManager::mainloop()
    {
        Time::start();
        float lastPhysicsUpdate = 0.f;
        float lastNetworkUpdate = 0.f;

        while ( !m_Done )
        {
            inputManager()->update();

            // Deform all components by type into a single array of array<of components>
            Map<u32, Vector<ComponentArray>>& allComponents = componentManager()->components();
            Vector<ComponentArray> sortedListOfComponentArrays;
            sortedListOfComponentArrays.reserve(allComponents.size());
            for ( auto& kvp : allComponents )
            {
                Vector<ComponentArray>& compArrayList = kvp.second;
                for ( auto& compArray : compArrayList )
                {
                    if ( compArray.size > 0 && compArray.elements[0].updatable() )
                    {
                        sortedListOfComponentArrays.emplace_back( compArray );
                    }
                }
            }

            // Sort update by priority
            Sort(sortedListOfComponentArrays, [](const ComponentArray& a, const ComponentArray& b)
            {
                assert( a.size && b.size );
                return a.elements[0].updatePriority() < b.elements[0].updatePriority();
            });

            // Call begin (ST)
            for ( auto& components : sortedListOfComponentArrays )
                for ( u32 i=0; i<components.size; ++i )
                {
                    Component* c = (Component*)((char*)components.elements + i*components.compSize);
                    if ( c->m_Active && !c->m_HasBegun )
                    {
                        c->m_HasBegun = true;
                        c->begin();
                    }
                }

            // MT updates first
            setExecutingParallel( true );

            // Check to see if can update network
            if ( Time::elapsed() - lastNetworkUpdate )
            {
                lastNetworkUpdate = Time::elapsed();
                for ( auto& components : sortedListOfComponentArrays )
                    for ( u32 i=0; i<components.size; ++i )
                    {
                        Component* c = (Component*) ((char*)components.elements + i*components.compSize);
                        if ( c->m_Active )
                            c->networkUpdateMT( Time::networkDt() );
                    }
            }

            // Check to see if can update physics
            if ( Time::elapsed() - lastPhysicsUpdate )
            {
                lastPhysicsUpdate = Time::elapsed();
                for ( auto& components : sortedListOfComponentArrays )
                    for ( u32 i=0; i<components.size; ++i )
                    {
                        Component* c = (Component*) ((char*)components.elements + i*components.compSize);
                        if ( c->m_Active )
                            c->physicsUpdateMT(Time::networkDt());
                    }
            }

            // Call MT update on components
            for ( auto& components : sortedListOfComponentArrays )
                for ( u32 i=0; i<components.size; ++i )
                {
                    Component* c = (Component*) ((char*)components.elements + i*components.compSize);
                    if ( c->m_Active )
                        c->updateMT(Time::networkDt());
                }

            setExecutingParallel( false );

            // Call ST update on components
            for ( auto& components : sortedListOfComponentArrays )
                for ( u32 i=0; i<components.size; ++i )
                {
                    Component* c = (Component*) ((char*)components.elements + i*components.compSize);
                    if ( c->m_Active )
                        c->updateMT(Time::networkDt());
                }

            if ( m_Window )
            {
                OSSwapWindow( m_Window );
            }

            // Update timings
            Time::update();
        }

        OSDestroyWindow( m_Window );
    }


    SystemManager* g_SystemManager {};
    SystemManager* systemManager() { return CreateOnce(g_SystemManager); }
    void deleteSystemManager() { delete g_SystemManager; g_SystemManager=nullptr; }
}
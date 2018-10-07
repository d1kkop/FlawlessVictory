#include "SystemManager.h"
#include "../Core/Algorithm.h"
#include "../Core/Time.h"
#include "../Core/Thread.h"
#include "../Core/OSLayer.h"
#include "../Core/InputManager.h"
#include "../Core/ComponentManager.h"
#include "../Scene/GameComponent.h"
#include "../Render/RenderManager.h"

namespace fv
{
    // For testing module only. TODO change this for something more appropriate.
    void SystemManager::callBeginForComponent(class GameComponent* gc)
    {
        gc->begin();
    }

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
        RenderManagerParams renderParams;
        renderParams.windowHandles.emplace_back( m_Window );
        if ( !renderManager()->initGraphics( renderParams ) )
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
            if (!inputManager()->update())
                break;

            // Deform all components by type into a single array of array<of components>
            Map<u32, Vector<ComponentArray>>& allComponents = componentManager()->components();
            Vector<ComponentArray> sortedListOfComponentArrays;
            sortedListOfComponentArrays.reserve(allComponents.size());
            for ( auto& kvp : allComponents )
            {
                Vector<ComponentArray>& compArrayList = kvp.second;
                for ( auto& compArray : compArrayList )
                {
                    if ( compArray.size > 0 && ((GameComponent&)compArray.elements[0]).updatable() )
                    {
                        sortedListOfComponentArrays.emplace_back( compArray );
                    }
                }
            }

            // Sort update by priority
            Sort(sortedListOfComponentArrays, [](const ComponentArray& a, const ComponentArray& b)
            {
                assert( a.size && b.size );
                return ((GameComponent&)a.elements[0]).updatePriority() < ((GameComponent&)b.elements[0]).updatePriority();
            });

            // Call begin (ST)
            for ( auto& components : sortedListOfComponentArrays )
                for ( u32 i=0; i<components.size; ++i )
                {
                    GameComponent* c = (GameComponent*)((char*)components.elements + i*components.compSize);
                    if ( c->inUse() && !c->m_HasBegun )
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
                        GameComponent* c = (GameComponent*) ((char*)components.elements + i*components.compSize);
                        if ( c->inUse() )
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
                        GameComponent* c = (GameComponent*) ((char*)components.elements + i*components.compSize);
                        if ( c->inUse() )
                            c->physicsUpdateMT(Time::networkDt());
                    }
            }

            // Call MT update on components
            for ( auto& components : sortedListOfComponentArrays )
                for ( u32 i=0; i<components.size; ++i )
                {
                    GameComponent* c = (GameComponent*) ((char*)components.elements + i*components.compSize);
                    if ( c->inUse() )
                        c->updateMT(Time::networkDt());
                }

            setExecutingParallel( false );

            // Call ST update on components
            for ( auto& components : sortedListOfComponentArrays )
                for ( u32 i=0; i<components.size; ++i )
                {
                    GameComponent* c = (GameComponent*) ((char*)components.elements + i*components.compSize);
                    if ( c->inUse() )
                        c->updateMT(Time::networkDt());
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
#include "SystemManager.h"
#include "../Core/Algorithm.h"
#include "../Core/Time.h"
#include "../Core/Thread.h"
#include "../Core/OSLayer.h"
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
        return true;
    }

    void SystemManager::mainloop()
    {
        Time::start();
        float lastPhysicsUpdate = 0.f;
        float lastNetworkUpdate = 0.f;

        while ( !m_Done )
        {
            // Sort components by priority
            Map<u32, Array<Component*>>& componentsList = componentManager()->updatableComponents();
            Array<Array<Component*>*> sortedList;
            sortedList.reserve(componentsList.size());
            for ( auto& kvp : componentsList )
                sortedList.emplace_back( &kvp.second );
            Sort(sortedList, [](const Array<Component*>* a, const Array<Component*>* b)
            {
                if ( a->size() && b->size() )
                {
                    return (*a)[0]->updatePriority() < (*b)[0]->updatePriority();
                }
                return false;
            });

            // MT updates first
            setExecutingParallel( true );

            // Check to see if can update network
            if ( Time::elapsed() - lastNetworkUpdate )
            {
                lastNetworkUpdate = Time::elapsed();
                for ( auto& components : sortedList )
                    for ( Component* c : *components )
                        c->networkUpdateMT( Time::networkDt() );
            }

            // Check to see if can update physics
            if ( Time::elapsed() - lastPhysicsUpdate )
            {
                lastPhysicsUpdate = Time::elapsed();
                for ( auto& components : sortedList )
                    for ( Component* c : *components )
                        c->physicsUpdateMT( Time::networkDt() );
            }

            // Call MT update on components
            for ( auto& components : sortedList )
                for ( Component* c : *components )
                    c->updateMT( Time::dt() );

            setExecutingParallel( false );

            // Call ST update on components
            for ( auto& components : sortedList )
                for ( Component* c : *components )
                    c->update( Time::dt() );

            // Update timings
            Time::update();
        }
    }


    SystemManager* g_SystemManager {};
    SystemManager* systemManager() { return CreateOnce(g_SystemManager); }
}
#include "SystemManager.h"
#include "../Core/Algorithm.h"
#include "../Core/Time.h"
#include "../Core/ComponentManager.h"
#include "../Core/Vec3.h"

namespace fv
{
    void SystemManager::initialize()
    {

    }

    void SystemManager::mainloop()
    {
        Time::start();
        float lastPhysicsUpdate = 0.f;
        float lastNetworkUpdate = 0.f;

        while ( !m_Done )
        {
            // Sort components by priority
            Map<u32, Array<Component*>>& componentsList = componentManager()->activeComponents();
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
            componentManager()->setExecutingSingleThreaded(false);

            // Check to see if can update network
            if ( Time::elapsed() - lastNetworkUpdate )
            {
                lastNetworkUpdate = Time::elapsed();
                for ( auto& components : sortedList )
                    for ( Component* c : *components )
                        if ( c->m_DoNetworkUpdate ) c->networkUpdateMT( Time::networkDt() );
            }

            // Check to see if can update physics
            if ( Time::elapsed() - lastPhysicsUpdate )
            {
                lastPhysicsUpdate = Time::elapsed();
                for ( auto& components : sortedList )
                    for ( Component* c : *components )
                        if ( c->m_DoPhysicsUpdate ) c->physicsUpdateMT( Time::networkDt() );
            }

            // Call MT update on components
            for ( auto& components : sortedList )
                for ( Component* c : *components )
                    if ( c->m_DoUpdate ) c->updateMT( Time::dt() );

            componentManager()->setExecutingSingleThreaded(true);

            // Call ST update on components
            for ( auto& components : sortedList )
                for ( Component* c : *components )
                    if ( c->m_DoUpdate ) c->updateST( Time::dt() );

            // Update timings
            Time::update();
        }
    }

}
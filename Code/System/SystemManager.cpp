#include "SystemManager.h"
#include "../Core/Algorithm.h"
#include "../Core/Time.h"
#include "../Core/Thread.h"
#include "../Core/OSLayer.h"
#include "../Core/InputManager.h"
#include "../Scene/GameComponent.h"
#include "../Render/RenderManager.h"
#include "../Resource/ResourceManager.h"

namespace fv
{
    bool SystemManager::initialize(const SystemParams& params)
    {
        m_Params = params;

        // Initilize os related stuff.
        if ( !OSInitialize() )
        {
            return false;
        }

        // Initialize graphics.
        if ( !renderManager()->initGraphics() )
        {
            return false;
        }

        // Load user provided module.
        OSHandle h = OSLoadLibrary(params.moduleName.c_str());
        if ( !h.library )
        {
            return false;
        }

        h = OSFindFunction(h, "entry");
        if ( h.function )
        {
            // Execute optional entry point of loaded module.
            using entryFunc = void (*)();
            entryFunc entry = (entryFunc)h.function;
            entry();
        }
        else
        {
            // This is optional. So just info about it but continue.
            LOG("Entry function not found in %s.", params.moduleName.c_str());
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

            // Deform all components by type into a single list of arrays_of_components sorted by update priority.
            Map<u32, Vector<ComponentArray>>& allComponents = componentManager()->components();
            m_SortedListOfComponentArrays.reserve(allComponents.size());
            for ( auto& kvp : allComponents )
            {
                Vector<ComponentArray>& compArrayList = kvp.second;
                for ( auto& compArray : compArrayList )
                {
                    if ( compArray.size > 0 && ((GameComponent&)compArray.elements[0]).m_DoUpdate )
                    {
                        m_SortedListOfComponentArrays.emplace_back( compArray );
                    }
                }
            }

            // Deformed, now sort it.
            Sort(m_SortedListOfComponentArrays, [](const ComponentArray& a, const ComponentArray& b)
            {
                assert( a.size && b.size );
                return ((GameComponent&)a.elements[0]).m_UpdatePriority < ((GameComponent&)b.elements[0]).m_UpdatePriority;
            });

            // Call begin (Single threaded).
            for ( auto& components : m_SortedListOfComponentArrays )
                for ( u32 i=0; i<components.size; ++i )
                {
                    GameComponent* c = (GameComponent*)((char*)components.elements + i*components.compSize);
                    if ( c->inUse() && !c->m_HasBegun )
                    {
                        c->m_HasBegun = true;
                        c->begin();
                    }
                }

            // Each frame, process resources after begin, so that all resources started with a load in 'begin' are loaded before the first update call.
            resourceManager()->processResources(); // This is in parallel (Multi threaded).

            // Check to see if can update network
            if ( Time::elapsed() - lastNetworkUpdate )
            {
                lastNetworkUpdate = Time::elapsed();
                for ( auto& components : m_SortedListOfComponentArrays )
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
                for ( auto& components : m_SortedListOfComponentArrays )
                    for ( u32 i=0; i<components.size; ++i )
                    {
                        GameComponent* c = (GameComponent*) ((char*)components.elements + i*components.compSize);
                        if ( c->inUse() )
                            c->physicsUpdateMT(Time::networkDt());
                    }
            }

            // Call MT update on components
            for ( auto& components : m_SortedListOfComponentArrays )
                for ( u32 i=0; i<components.size; ++i )
                {
                    GameComponent* c = (GameComponent*) ((char*)components.elements + i*components.compSize);
                    if ( c->inUse() )
                        c->updateMT(Time::networkDt());
                }

            // Call ST update on components
            for ( auto& components : m_SortedListOfComponentArrays )
                for ( u32 i=0; i<components.size; ++i )
                {
                    GameComponent* c = (GameComponent*) ((char*)components.elements + i*components.compSize);
                    if ( c->inUse() )
                        c->updateMT(Time::networkDt());
                }

            // Update timings
            Time::update();
        }
    }


    SystemManager* g_SystemManager {};
    SystemManager* systemManager() { return CreateOnce(g_SystemManager); }
    void deleteSystemManager() { delete g_SystemManager; g_SystemManager=nullptr; }
}
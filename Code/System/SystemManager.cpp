#include "SystemManager.h"
#include "../Core/Algorithm.h"
#include "../Core/Time.h"
#include "../Core/Thread.h"
#include "../Core/OSLayer.h"
#include "../Core/InputManager.h"
#include "../Core/JobManager.h"
#include "../Scene/GameComponent.h"
#include "../Render/RenderManager.h"
#include "../Resource/PatchManager.h"
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

        // Execute functions/constructors that are thread safe in a sense that their data will never change.
        // Call these before any threads are started.
        resourceManager()->initialize();

        // Initialize graphics.
        if ( !renderManager()->initGraphics() )
        {
            return false;
        }

        if ( !inputManager()->initialize( renderManager()->getWindowHandle() ) )
        {
            return false;
        }

        StopBeginFase();

        // Load user provided module.
        OSHandle h = OSLoadLibrary(params.moduleName.c_str());
        if ( h.invalid() )
        {
            return false;
        }

        void* function = OSFindFunction(h, "entry");
        if ( function )
        {
            // Execute optional entry point of loaded module.
            using entryFunc = void (*)(i32, char**);
            entryFunc entry = (entryFunc)function;
            entry( params.argc, params.argv );
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
        TimeStart();
        float lastPhysicsUpdate = 0.f;
        float lastNetworkUpdate = 0.f;

        while ( !m_Done )
        {
            // Apply patches from background processing onto main thread.
            patchManager()->applyPatches();

            if (!inputManager()->update())
                break;

            if ( KeyPressed(Keyboard::Escape) )
                m_Done = true;

           PrepareList(componentManager()->updateComponents(), m_ListUpdatables, [](const ComponentArray& a, const ComponentArray& b)
           {
                assert(a.size && b.size);
                return ((GameComponent&)a.elements[0]).m_UpdatePriority < ((GameComponent&)b.elements[0]).m_UpdatePriority;
           });

            // Call begin (Single threaded).
            ComponentFor<GameComponent>( m_ListUpdatables, [](GameComponent& gc)
            {
                if ( !gc.m_HasBegun )
                {
                    gc.m_HasBegun = true;
                    gc.begin();
                }
            });

            // Check to see if can update network
            if ( Time::elapsed() - lastNetworkUpdate )
            {
                lastNetworkUpdate = Time::elapsed();
                PrepareList( componentManager()->networkComponents(), m_ListOthers );
                /*  ParallelComponentFor<GameComponent>( m_ListOthers, []( GameComponent& gc)
                  {
                      gc.networkUpdateMT( Time::networkDt() );
                  });*/
            }

            // Check to see if can update physics
            if ( Time::elapsed() - lastPhysicsUpdate )
            {
                lastPhysicsUpdate = Time::elapsed();
                PrepareList( componentManager()->physicsComponents(), m_ListOthers );
  /*              ParallelComponentFor<GameComponent>( m_ListOthers, [](GameComponent& gc)
                {
                    gc.physicsUpdateMT(Time::physicsDt());
                });*/
            }

            // Call MT update on components
            ParallelComponentFor<GameComponent>( m_ListUpdatables, [](GameComponent& gc, u32 tIdx)
            {
                gc.updateMT( Time::networkDt() );
            });

            // Call ST update on components
            ComponentFor<GameComponent>( m_ListUpdatables, [](GameComponent& gc)
            {
                gc.update( Time::dt() );
            });


            Mat4 view = Mat4::identity();
            renderManager()->render();

            // Update timings
            TimeUpdate();
        }

    }

    SystemManager* g_SystemManager {};
    SystemManager* systemManager() { return CreateOnce(g_SystemManager); }
    void deleteSystemManager() { delete g_SystemManager; g_SystemManager=nullptr; }
}
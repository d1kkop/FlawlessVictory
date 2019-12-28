#include "PCH.h"
#include "DestructionManager.h"
#include "Functions.h"
#include "OSLayer.h"
#include "LogManager.h"

namespace fv
{
    DestructionManager* g_DestructionManager{};
    DestructionManager* destructionManager() { return CreateOnce( g_DestructionManager ); }
    void deleteDestructionManager() { delete g_DestructionManager; g_DestructionManager=nullptr; }

    DestructionManager::DestructionManager()
    {
        m_DestructionThread = std::thread( [this]()
        {
            OSSetThreadName("DestructionThread");
            ThreadLoop();
        });
    }

    DestructionManager::~DestructionManager()
    {
        m_Closing = true;
        if ( m_DestructionThread.joinable() )
        {
            m_DestructionThread.join();
        }
        clearListOfDestructables();
    }

    FV_TS void DestructionManager::add( const M<Destructable>& destructable )
    {
        Lock lk(m_ListMutex);
        m_Destructables.emplace_back( destructable );
    }

    FV_TS void DestructionManager::clearListOfDestructables()
    {
        Lock lk(m_ListMutex);
        m_Destructables.clear();
    }

    void DestructionManager::ThreadLoop()
    {
        while (!m_Closing)
        {
            if ( m_ListMutex.try_lock() )
            {
                try
                {
                    for ( u32 i = 0; i < m_Destructables.size(); )
                    {
                        if ( m_Destructables[i].use_count() == 1 )
                        {
                            m_Destructables.erase( m_Destructables.begin() + i );
                            continue;
                        }
                        i++;
                    }
                }
                catch ( std::exception e )
                {
                    LOGW( "Exception in DestructionManager: %s.", e.what() );
                }
                m_ListMutex.unlock();
            }
            Suspend( 0.1 );
        }
    }

}
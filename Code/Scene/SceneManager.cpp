#include "SceneManager.h"
#include "SceneList.h"
#include "SceneComponent.h"
#include "../Core/Thread.h"
#include "../Core/LogManager.h"
#include "../Core/Functions.h"
#include "../Core/GameObject.h"

namespace fv
{
    u64 SceneManager::addScene(const String& name)
    {
        FV_CHECK_MO();
        for ( u32 i=0; i<64; ++i )
        {
            if ( !m_SceneLists[i].valid() )
            {
                m_SceneLists[i].m_Valid = true;
                m_SceneLists[i].m_Bit   = (1ULL<<i);
                m_SceneLists[i].m_Name  = name;
                return m_SceneLists[i].m_Bit;
            }
        }
        LOGW( "Cannot add new scene %s. Max scenes (64) reached.", name.c_str() );
        return -1;
    }

    void SceneManager::removeScene(const String& name)
    {
        FV_CHECK_MO();
    }

    void SceneManager::serialize( const String& name, const Path& filename, bool save)
    {
        FV_CHECK_MO();
        SceneList* sl = nullptr;
        for ( auto& s : m_SceneLists ) 
        {
            if ( s.valid() && s.m_Name == name )
            {
                sl = &s;
                break;
            }
        }
        if (!sl) 
        {
            LOGW("Cannot serialize %s, scene not found or not valid.", name.c_str() );
            return;
        }
        assert(sl);
        m_SceneComponentIdCounter = 0;
        m_Comp2Id.clear();
        m_Id2Comp.clear();
        sl->serialize( filename, save );
        if ( !save )
        {
            for ( auto& kvp : m_Id2Comp )
            {
                auto* c = kvp.second;
                if ( m_Comp2Id.count( c ) != 0 ) // find parentId
                {
                    u32 parentId = m_Comp2Id[c];
                    if ( m_Id2Comp.count( parentId ) != 0 )
                    {
                        c->attach( m_Id2Comp[parentId] );
                    }
                    else
                    {
                        LOGW("Failed to obtain parent for component %d.", kvp.first);
                    }
                }
            }
            for ( auto& kvp : m_Id2Comp )
            {
                auto* c = kvp.second;
                c->sceneBits() |= sl->m_Bit;
                c->computeLocalToWorld();
                c->computeWorldToLocal();
            }
        }
        m_Comp2Id.clear();
        m_Id2Comp.clear();
    }

    u32 SceneManager::getIdFor(SceneComponent* comp)
    {
        FV_CHECK_MO();
        if ( !comp ) return -1;
        if ( m_Comp2Id.count(comp) == 0 )
        {
            m_Comp2Id[comp] = nextSceneComponentId();
        }
        return m_Comp2Id[comp];
    }

    void SceneManager::setIdFor(SceneComponent* comp, u32 id )
    {
        FV_CHECK_MO();
        assert( id != -1 );
        if ( id==-1 )
        {
            LOGW("Invalid id %d for component.");
            return;
        }
        if ( m_Id2Comp.count( id ) == 0 )
        {
            m_Id2Comp[ id ] = comp;
        }
        else
        {
            LOGW("Multiple components with same id %d found, this is invalid. Fix by searching the id in the scene file and change to a number not in use (32 bits) and not -1.", id);
        }
    }

    void SceneManager::setParentIdFor(SceneComponent* comp, u32 parentId)
    {
        FV_CHECK_MO();
        if ( parentId == -1 ) return;
        if ( m_Comp2Id.count(comp) == 0 )
        {
            m_Comp2Id[comp] = parentId;
        }
    }

    SceneManager* g_SceneManager {};
    SceneManager* sceneManager() { return CreateOnce(g_SceneManager); }
    void deleteSceneManager() { delete g_SceneManager; g_SceneManager=nullptr; }


    GameObject* NewGameObject(u64 sceneMask)
    {
        FV_CHECK_MO();
        auto* go = gameObjectManager()->newObject();
        if ( !go ) return nullptr;
        if ( sceneMask!=0 )
        {
            auto sc = go->addComponent<SceneComponent>();
            sc->sceneBits() |= sceneMask;
        }
        return go;
    }
}
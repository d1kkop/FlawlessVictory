#include "SceneManager.h"
#include "SceneList.h"
#include "../Core/Thread.h"
#include "../Core/LogManager.h"
#include "../Core/Functions.h"

namespace fv
{
    SceneList SceneManager::addScene(const String& name)
    {
        FV_CHECK_MO();
        for ( u32 i=0; i<64; ++i )
        {
            if ( !m_SceneLists[i].valid() )
            {
                m_SceneLists[i].m_Valid = true;
                m_SceneLists[i].m_Bit   = (1ULL<<i);
                m_SceneLists[i].m_Name  = name;
                return m_SceneLists[i];
            }
        }
        LOGW( "Cannot add new scene %s. Max scenes (64) reached.", name.c_str() );
        return {};
    }

    void SceneManager::removeScene(const String& name)
    {
        FV_CHECK_MO();
    }

    void SceneManager::serialize( const Path& filename, bool save)
    {

    }

    SceneManager* g_SceneManager {};
    SceneManager* sceneManager() { return CreateOnce(g_SceneManager); }
    void deleteSceneManager() { delete g_SceneManager; g_SceneManager=nullptr; }
}
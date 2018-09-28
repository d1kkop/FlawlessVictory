#include "SceneWorld.h"

namespace fv
{
    void SceneWorld::addSceneComponent(SceneComponent* component)
    {
        m_Components.insert( component );
    }

    void SceneWorld::removeSceneComponent(SceneComponent* component)
    {
        auto cIt = m_Components.find( component );
        if ( cIt  != m_Components.end () )
        {
            m_Components.erase( cIt );
        }
    }
}
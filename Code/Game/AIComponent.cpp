#include "AIComponent.h"
#include "PlayerComponent.h"
#include "../Resource/ResourceManager.h"

namespace game
{
    FV_TYPE_IMPL(AIComponent)

    void AIComponent::begin()
    {
        m_FragShader = resourceManager()->load<Shader>( "sample.frag" );
        m_VertShader = resourceManager()->load<Shader>( "sample.vert" );
    }

    void AIComponent::updateMT(float t)
    {

    }

    void AIComponent::update(float t)
    {
        // Find closest player
        float dist = FLT_MAX;
        Vec3 usPos = getComponent<SceneComponent>()->position();
        PlayerComponent* closestPlayer = nullptr;
        for ( PlayerComponent& player : Itr<PlayerComponent>() )
        {
            Vec3 pos = player.getComponent<SceneComponent>()->position();
            float kDist = pos.distSq( usPos );
            if ( kDist < dist )
            {
                dist = kDist;
                closestPlayer = &player;
            }
        }
        if ( closestPlayer )
        {
            m_TargetPlayer = closestPlayer;
        }

        if ( m_TargetPlayer )
        {
           
        }
    }

}
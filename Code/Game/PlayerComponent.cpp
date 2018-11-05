#include "PlayerComponent.h"

namespace game
{
    FV_TYPE_IMPL(PlayerComponent)

    void PlayerComponent::serialize(TextSerializer& ts)
    {
        m_FragShader = resourceManager()->load<Shader>("sample.frag");
        m_VertShader = resourceManager()->load<Shader>("sample.vert");
    }

    void PlayerComponent::updateMT(float t)
    {
 
    }

    void PlayerComponent::update(float t)
    {
        // Update this component or other components in the owning game object.
  /*      for ( PlayerComponent* player : Itr<PlayerComponent>() )
        {

        }*/

        //ComponentCollection<PlayerComponent> it = Itr<PlayerComponent>();
        //auto bg = it.begin();
        //auto eg = it.end();
        //for ( ; bg != eg; ++bg )
        //{

        //}
    }

}
#include "PlayerComponent.h"

namespace game
{
    FV_TYPE_IMPL(PlayerComponent)

    void PlayerComponent::serialize(TextSerializer& ts)
    {

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
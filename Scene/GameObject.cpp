#include "GameObject.h"

namespace fv
{
    GameObject::GameObject()
    {
        // Placement new invokes this each time a game object is recycled.
        m_Components.clear();
    }

    u32 GameObject::numComponents()
    {
        FV_CHECK_ST();
        return (u32)m_Components.size();
    }
}
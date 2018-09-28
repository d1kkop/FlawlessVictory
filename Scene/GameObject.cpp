#include "GameObject.h"

namespace fv
{
    u32 GameObject::numComponents()
    {
        return (u32)m_Components.size();
    }
}
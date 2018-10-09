#include "GameComponent.h"
#include "GameObject.h"

namespace fv
{
    GameComponent* GameComponent::addComponent(u32 type)
    {
        assert(m_GameObject);
        return m_GameObject->addComponent(type);
    }

    GameComponent* GameComponent::getComponent(u32 type)
    {
        assert(m_GameObject);
        return m_GameObject->getComponent(type);
    }

    bool GameComponent::hasComponent(u32 type)
    {
        assert(m_GameObject);
        return m_GameObject->hasComponent(type);
    }

    bool GameComponent::removeComponent(u32 type)
    {
        assert(m_GameObject);
        return m_GameObject->removeComponent(type);
    }
}
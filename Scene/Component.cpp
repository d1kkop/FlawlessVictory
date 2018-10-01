#include "Component.h"
#include "GameObject.h"

namespace fv
{
    Component* Component::addComponent(u32 type)
    {
        assert(m_GameObject);
        return m_GameObject->addComponent(type);
    }

    Component* Component::getComponent(u32 type)
    {
        assert(m_GameObject);
        return m_GameObject->getComponent(type);
    }

    bool Component::hasComponent(u32 type)
    {
        assert(m_GameObject);
        return m_GameObject->hasComponent(type);
    }

    bool Component::removeComponent(u32 type)
    {
        assert(m_GameObject);
        return m_GameObject->removeComponent(type);
    }
}
#include "PCH.h"
#include "Component.h"
#include "GameObject.h"
#include "Thread.h"

namespace fv
{
    FV_MO M<Component> Component::addComponent(u32 type)
    {
        FV_CHECK_MO();
        assert(m_GameObject);
        return m_GameObject->addComponent(type);
    }

    FV_MO bool Component::removeComponent( u32 type )
    {
        FV_CHECK_MO();
        assert( m_GameObject );
        return m_GameObject->removeComponent( type );
    }

    M<Component> Component::getComponent(u32 type)
    {
        assert(m_GameObject);
        return m_GameObject->getComponent(type);
    }

    bool Component::hasComponent(u32 type) const
    {
        assert(m_GameObject);
        return m_GameObject->hasComponent(type);
    }

    GameObject* Component::gameObject() const
    {
        assert(m_GameObject);
        return m_GameObject;
    }
}
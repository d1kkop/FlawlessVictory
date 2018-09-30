#include "GameObject.h"

namespace fv
{
    GameObject::GameObject()
    {
        // Placement new invokes this each time a game object is recycled.
        m_Components.clear();
    }

    FV_ST Component* GameObject::addComponent(u32 type)
    {
        FV_CHECK_ST();
        Component* c = getComponent(type);
        if ( c ) 
        {
            LOGW("Component of type %s already added.", typeManager()->typeInfo(type).name->c_str() );
            return c;
        }
        c = componentManager()->newComponent( type );
        m_Components[type] = c;
        return c;
    }

    FV_ST Component* GameObject::getComponent(u32 type)
    {
        FV_CHECK_ST();
        auto cIt = m_Components.find( type );
        if ( cIt != m_Components.end() )
        {
            return cIt->second;
        }
        return nullptr;
    }

    FV_ST bool GameObject::hasComponent(u32 type)
    {
        FV_CHECK_ST();
        return m_Components.count(type)!=0;
    }

    FV_ST bool GameObject::removeComponent(u32 type)
    {
        FV_CHECK_ST();
        auto cIt = m_Components.find(type);
        if ( cIt != m_Components.end() )
        {
            componentManager()->freeComponent( cIt->second );
            m_Components.erase( cIt );
            return true;
        }
        return false;
    }

    u32 GameObject::numComponents()
    {
        FV_CHECK_ST();
        return (u32)m_Components.size();
    }
}
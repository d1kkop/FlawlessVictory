#include "GameObject.h"
#include "SceneComponent.h"
#include "../Core/Functions.h"
#include "../Core/Reflection.h"
#include "../Core/ObjectManager.h"

namespace fv
{
    GameObject::GameObject()
    {
        // Placement new invokes this each time a game object is recycled.
        m_Components.clear();
    }

    FV_MO GameComponent* GameObject::addComponent(u32 type)
    {
        FV_CHECK_MO();
        GameComponent* c = getComponent(type);
        if ( c ) 
        {
            auto* ti = typeManager()->typeInfo(type);
            LOGW("Component of type %s already added.", ti?ti->name->c_str():"unknown" );
            return c;
        }
        c = sc<GameComponent*>(componentManager()->newComponent( type ));
        m_Components[type] = c;
        c->m_GameObject = this;
        return c;
    }

    FV_MO GameComponent* GameObject::getComponent(u32 type)
    {
        FV_CHECK_MO();
        auto cIt = m_Components.find( type );
        if ( cIt != m_Components.end() )
        {
            return cIt->second;
        }
        return nullptr;
    }

    FV_MO bool GameObject::hasComponent(u32 type)
    {
        FV_CHECK_MO();
        return m_Components.count(type)!=0;
    }

    FV_MO bool GameObject::removeComponent(u32 type)
    {
        FV_CHECK_MO();
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
        FV_CHECK_MO();
        return (u32)m_Components.size();
    }

    ObjectManager<GameObject>* g_GameObjectManager {};
    ObjectManager<GameObject>* gameObjectManager() { return CreateOnce(g_GameObjectManager); }
    void deleteGameObjectManager() { delete g_GameObjectManager; g_GameObjectManager=nullptr; }

    GameObject* NewGameObject(bool addSceneComponent)
    {
        FV_CHECK_MO();
        auto* go = gameObjectManager()->newObject();
        if ( addSceneComponent ) go->addComponent<SceneComponent>();
        return go;
    }
}
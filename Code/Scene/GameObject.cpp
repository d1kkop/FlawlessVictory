#include "GameObject.h"
#include "SceneComponent.h"
#include "../Core/TextSerializer.h"
#include "../Core/Functions.h"
#include "../Core/Reflection.h"
#include "../Core/SparseArray.h"

namespace fv
{
    GameObject::GameObject()
    {
        // Placement new invokes this each time a game object is recycled.
        m_Components.clear();
    }

    GameComponent* GameObject::addComponent(u32 type)
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
        if ( c )
        { // Is null if type no longer exists
            m_Components[type] = c;
            c->m_GameObject = this;
        }
        return c;
    }

    GameComponent* GameObject::getComponent(u32 type)
    {
        FV_CHECK_MO();
        auto cIt = m_Components.find( type );
        if ( cIt != m_Components.end() )
        {
            return cIt->second;
        }
        return nullptr;
    }

    bool GameObject::hasComponent(u32 type)
    {
        FV_CHECK_MO();
        return m_Components.count(type)!=0;
    }

    bool GameObject::removeComponent(u32 type)
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

    void GameObject::removeAllComponents()
    {
        FV_CHECK_MO();
        for ( auto& kvp : m_Components )
        {
            componentManager()->freeComponent( kvp.second );
        }
        m_Components.clear();
    }

    u32 GameObject::numComponents()
    {
        FV_CHECK_MO();
        return (u32)m_Components.size();
    }

    u32 GameObject::prototypeId() const
    {
        FV_CHECK_MO();
        return m_PrototypeId;
    }

    const String& GameObject::name() const
    {
        FV_CHECK_MO();
        return m_Name;
    }

    u32 GameObject::id() const
    {
        FV_CHECK_MO();
        return m_Id;
    }

    void GameObject::serialize(TextSerializer& ts)
    {
        FV_CHECK_MO();
        ts.serialize( "id", m_Id );
        for ( auto& kvp : m_Components )
        {
            const TypeInfo* ti = typeManager()->typeInfo( kvp.second->type() );
            if ( !ti ) continue;
            ts.pushObject();
                kvp.second->serialize( ts );
            ts.popObject( *ti->name );
        }
    }

    SparseArray<GameObject>* g_GameObjectManager {};
    SparseArray<GameObject>* gameObjectManager() { return CreateOnce(g_GameObjectManager); }
    void deleteGameObjectManager() { delete g_GameObjectManager; g_GameObjectManager=nullptr; }

    GameObject* NewGameObject(bool addSceneComponent)
    {
        FV_CHECK_MO();
        auto* go = gameObjectManager()->newObject();
        if ( addSceneComponent ) go->addComponent<SceneComponent>();
        return go;
    }

    void DestroyGameObject(GameObject* go)
    {
        FV_CHECK_MO();
        if ( !go ) return;
        go->removeAllComponents();
        gameObjectManager()->freeObject( go );
    }

}
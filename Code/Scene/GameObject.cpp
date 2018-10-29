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

    void GameObject::serialize(TextSerializer& ts)
    {
        FV_CHECK_MO();
        if ( ts.isWriting() )
        {
            ts.pushArray("components");
            for ( auto& kvp : m_Components )
            {
                ts.beginArrayElement();
                u32 type = kvp.second->type();
                auto ti  = typeManager()->typeInfo(type);
                if (ti)
                {
                    String name = *ti->name;
                    ts.serialize( "type", type );
                    ts.serialize( "name", name );
                    kvp.second->serialize( ts );
                }
                ts.endArrayElement();
            }
            ts.popArray();
        }
        else
        {
            ts.pushArray("components");
            while ( ts.beginArrayElement() )
            {
                u32 compType;
                String name;
                ts.serialize("type", compType);
                const TypeInfo* ti = typeManager()->typeInfo( compType );
                if (ti)
                {
                    auto* c = addComponent(compType);
                    if (c) c->serialize( ts );
                }
                ts.endArrayElement();
            }
            ts.popArray();
        }
    }

    SparseArray<GameObject>* g_GameObjectManager {};
    SparseArray<GameObject>* gameObjectManager() { return CreateOnce(g_GameObjectManager); }
    void deleteGameObjectManager() { delete g_GameObjectManager; g_GameObjectManager=nullptr; }

    GameObject* NewGameObject(u64 sceneMask)
    {
        FV_CHECK_MO();
        auto* go = gameObjectManager()->newObject();
        if (!go) return nullptr;
        if ( sceneMask!=0 ) 
        {
            SceneComponent* sc = go->addComponent<SceneComponent>();
            sc->sceneBits() |= sceneMask;
        }
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
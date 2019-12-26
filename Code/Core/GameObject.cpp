#include "PCH.h"
#include "GameObject.h"
#include "TextSerializer.h"
#include "Functions.h"
#include "Reflection.h"
#include "SparseArray.h"
#include "TransformManager.h"

namespace fv
{
    FV_MO GameObject::GameObject() :
        m_LocalToWorld( transformManager()->newLocalToWorldMatrix() ),
        m_WorldToLocal( transformManager()->newWorldToLocalMatrix() )
    {
        FV_CHECK_MO();
    }

    FV_MO GameObject::~GameObject()
    {
        FV_CHECK_MO();
        transformManager()->freeLocalToWorldMatrix( m_LocalToWorld );
        transformManager()->freeWorldToLocalMatrix( m_WorldToLocal );
        m_Components.clear();
    }

    FV_MO M<Component> GameObject::addComponent( u32 type )
    {
        FV_CHECK_MO();
        auto c = getComponent( type );
        if ( c )
        {
            auto* ti = typeManager()->typeInfo( type );
            LOGW( "Component of type %s already added.", ti?ti->name->c_str():"unknown" );
            return c;
        }
        c = componentManager()->newComponent( type );
        if ( c )
        { // Is null if type no longer exists
            m_Components[type] = c;
            c->m_GameObject = this;
        }
        return c;
    }

    FV_MO FV_DLL M<Component> GameObject::addOrGetComponent( u32 type )
    {
        FV_CHECK_MO();
        auto c = getComponent( type );
        if ( c ) return c;
        return addComponent( type );
    }

    FV_MO FV_DLL bool GameObject::removeComponent( u32 type )
    {
        FV_CHECK_MO();
        auto cIt = m_Components.find( type );
        if ( cIt != m_Components.end() )
        {
            m_Components.erase( cIt );
            return true;
        }
        return false;
    }

    FV_DLL M<Component> GameObject::getComponent( u32 type )
    {
        auto cIt = m_Components.find( type );
        if ( cIt != m_Components.end() )
        {
            return cIt->second;
        }
        return nullptr;
    }

    FV_DLL bool GameObject::hasComponent( u32 type ) const
    {
        return m_Components.count( type )!=0;
    }

    FV_MO FV_DLL void GameObject::removeAllComponents()
    {
        FV_CHECK_MO()
        m_Components.clear();
    }

    u32 GameObject::numComponents()
    {
        return (u32)m_Components.size();
    }

    FV_MO void GameObject::serialize( TextSerializer& ts )
    {
        FV_CHECK_MO();
        if ( ts.isWriting() )
        {
            ts.pushArray( "components" );
            for ( auto& kvp : m_Components )
            {
                ts.beginArrayElement();
                u32 type = kvp.second->type();
                auto ti  = typeManager()->typeInfo( type );
                if ( ti )
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
            ts.pushArray( "components" );
            while ( ts.beginArrayElement() )
            {
                u32 compType;
                String name;
                ts.serialize( "type", compType );
                const TypeInfo* ti = typeManager()->typeInfo( compType );
                if ( ti )
                {
                    auto c = addComponent( compType );
                    if ( c ) c->serialize( ts );
                }
                ts.endArrayElement();
            }
            ts.popArray();
        }
    }

    FV_MO const String& GameObject::name() const
    {
        FV_CHECK_MO();
        return m_Name;
    }

    FV_TC u32 GameObject::prototypeId() const
    {
        return m_PrototypeId;
    }

    SparseArray<GameObject>* g_GameObjectManager{};
    SparseArray<GameObject>* gameObjectManager() { return CreateOnce( g_GameObjectManager ); }
    void deleteGameObjectManager() { delete g_GameObjectManager; g_GameObjectManager=nullptr; }

    void DestroyGameObject( GameObject* go )
    {
        FV_CHECK_MO();
        if ( !go ) return;
        go->removeAllComponents();
        gameObjectManager()->freeObject( go );
    }

}
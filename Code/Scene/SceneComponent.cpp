#include "SceneComponent.h"
#include "GameObject.h"
#include "SceneManager.h"
#include "../Core/Algorithm.h"
#include "../Core/Math.h"
#include "../Core/Thread.h"
#include "../Core/Reflection.h"
#include "../Core/TextSerializer.h"
#include <cassert>

namespace fv
{
    FV_TYPE_IMPL(SceneComponent)

    void SceneComponent::move(const Vec3& translate)
    {
        m_LocalToWorldDirty = true;
        m_Position += translate;
    }

    void SceneComponent::setPosition(const Vec3& position)
    {
        m_LocalToWorldDirty = true;
        m_Position = position;
    }

    const fv::Vec3& SceneComponent::position() const
    {
        return m_Position;
    }

    void SceneComponent::rotate(const Quat& rotate)
    {
        m_LocalToWorldDirty = true;
        m_Rotation = rotate * m_Rotation;
    }

    void SceneComponent::setRotation(const Quat& rotation)
    {
        m_LocalToWorldDirty = true;
        m_Rotation = rotation;
    }

    const Quat& SceneComponent::rotation() const
    {
        return m_Rotation;
    }

    void SceneComponent::scale(const Vec3& scale)
    {
        m_LocalToWorldDirty = true;
        m_Scale *= scale;
    }

    void SceneComponent::setScale(const Vec3& scale)
    {
        m_LocalToWorldDirty = true;
        m_Scale = scale;
    }

    const fv::Vec3& SceneComponent::scale() const
    {
        return m_Scale;
    }

    void SceneComponent::attach(SceneComponent* other)
    {
        FV_CHECK_MO();
        detachFromParent();
        if ( other )
        {
            if ( !inHierarchy(other) )
            {
                other->m_Children.push_back( this );
                m_Parent = other;
            }
            else
            {
                LOGW("Cannot attach scene component to component that is already in the hierarchy. Attach failed.");
            }
        }
    }

    void SceneComponent::detachFromParent()
    {
        FV_CHECK_MO();
        if ( m_Parent )
        {
            Remove(m_Parent->m_Children, this);
            m_Parent = nullptr;
            computeLocalToWorld();
            computeTRSWorldToLocal();
        }
    }

    void SceneComponent::detachChildren()
    {
        FV_CHECK_MO();
        for ( auto* c : m_Children )
        {
            c->computeLocalToWorld();
            c->computeTRSWorldToLocal();
            c->m_Parent = nullptr;
        }
        m_Children.clear();
    }

    bool SceneComponent::inHierarchy(const SceneComponent* other) const
    {
        if ( other == nullptr ) return false;
        const SceneComponent* root = this;
        while ( root->m_Parent ) 
            root = root->m_Parent;
        if ( root == other ) return true;
        return isChildOrGrandChild( other );
    }

    bool SceneComponent::isChildOrGrandChild(const SceneComponent* other) const
    {
        if (!other) return false;
        for ( auto* c : m_Children )
        {
            if ( c == other ) return true;
            if ( c->isChildOrGrandChild(other) ) return true;
        }
        return false;
    }

    void SceneComponent::serialize(TextSerializer& ts)
    {
        FV_CHECK_MO();
        if ( ts.isWriting() )
        {
            u32 parentId = sceneManager()->getIdFor( m_Parent );
            u32 selfId = sceneManager()->getIdFor( this );
            ts.serialize( "parent", parentId );
            ts.serialize( "self", selfId );
        }
        else
        {
            u32 parentId, selfId;
            ts.serialize("parent", parentId);
            ts.serialize("self", selfId);
            sceneManager()->setIdFor( this, selfId );
            sceneManager()->setParentIdFor( this, parentId );
        }
        ts.serialize( "position", m_Position );
        ts.serialize( "quat", m_Rotation );
        ts.serialize( "scale", m_Scale );
    }

    void SceneComponent::computeLocalToWorld()
    {
        FV_CHECK_MO();
        if ( m_LocalToWorldDirty || (m_Parent && m_Parent->m_LocalToWorldDirty) )
        {
            Mat4 TRS = Mat4::compose(m_Position, m_Rotation, m_Scale);
            if ( m_Parent )
            {
                m_Parent->computeLocalToWorld();
                m_LocalToWorld = m_Parent->m_LocalToWorld * TRS;
            }
            else
            {
                m_LocalToWorld = TRS;
            }
            m_LocalToWorldDirty = false;
        }
    }

    void SceneComponent::computeWorldToLocal()
    {
        FV_CHECK_MO();
        computeLocalToWorld();
        if ( m_WorldToLocalDirty )
        {
            m_WorldToLocalDirty = false;
            m_WorldToLocal = m_LocalToWorld.inverse();
        }
    }

    void SceneComponent::computeTRSWorldToLocal()
    {
        FV_CHECK_MO();
        computeWorldToLocal();
        m_WorldToLocal.decompose(m_Position, m_Rotation, m_Scale);
    }

}
#include "SceneComponent.h"
#include "SceneManager.h"
#include "../Core/Algorithm.h"
#include "../Core/Math.h"
#include "../Core/Thread.h"
#include "../Core/TextSerializer.h"
#include "../Core/TransformManager.h"
#include <cassert>

#define FV_SCENECOMPONENT_PRIORITY (1000)

namespace fv
{
    FV_TYPE_IMPL(SceneComponent)

    SceneComponent::SceneComponent()
    {
        FV_CHECK_MO();
        m_UpdatePriority = FV_SCENECOMPONENT_PRIORITY;
    }

    SceneComponent::~SceneComponent()
    {
        FV_CHECK_MO();
    }

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

    const Vec3& SceneComponent::scale() const
    {
        return m_Scale;
    }

    bool SceneComponent::attach(SceneComponent* other)
    {
        FV_CHECK_MO();
        detachFromParent();
        if ( other && !inHierarchy(other) )
        {
            other->m_Children.push_back( this );
            m_Parent = other;
            return true;
        }
        return false;
    }

    void SceneComponent::detachFromParent()
    {
        FV_CHECK_MO();
        if ( m_Parent )
        {
            computeLocalToWorld();
            computeTRSFromWorldToLocal();
            Remove(m_Parent->m_Children, this);
            m_Parent = nullptr;
        }
    }

    void SceneComponent::detachChildren()
    {
        FV_CHECK_MO();
        for ( auto* c : m_Children )
        {
            c->computeLocalToWorld();
            c->computeTRSFromWorldToLocal();
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
                localToWorld() = m_Parent->localToWorld() * TRS;
            }
            else
            {
                localToWorld() = TRS;
            }
            m_LocalToWorldDirty = false;
        }
    }

    void SceneComponent::computeWorldToLocal()
    {
        FV_CHECK_MO();
        computeLocalToWorld();
        worldToLocal() = localToWorld().inverse();
    }

    void SceneComponent::computeTRSFromWorldToLocal()
    {
        FV_CHECK_MO();
        computeWorldToLocal();
        worldToLocal().decompose(m_Position, m_Rotation, m_Scale);
    }

}
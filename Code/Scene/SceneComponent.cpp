#include "SceneComponent.h"
#include "GameObject.h"
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
        m_MatrixDirty = true;
        m_Position += translate;
    }

    void SceneComponent::setPosition(const Vec3& position)
    {
        m_MatrixDirty = true;
        m_Position = position;
    }

    const fv::Vec3& SceneComponent::position() const
    {
        return m_Position;
    }

    void SceneComponent::rotate(const Quat& rotate)
    {
        m_MatrixDirty = true;
        m_Rotation = rotate * m_Rotation;
    }

    void SceneComponent::setRotation(const Quat& rotation)
    {
        m_MatrixDirty = true;
        m_Rotation = rotation;
    }

    const Quat& SceneComponent::rotation() const
    {
        return m_Rotation;
    }

    void SceneComponent::scale(const Vec3& scale)
    {
        m_MatrixDirty = true;
        m_Scale *= scale;
    }

    void SceneComponent::setScale(const Vec3& scale)
    {
        m_MatrixDirty = true;
        m_Scale = scale;
    }

    const fv::Vec3& SceneComponent::scale() const
    {
        return m_Scale;
    }

    void SceneComponent::attach(SceneComponent* other)
    {
        FV_CHECK_MO();
        detachSelf();
        if ( other )
        {
            if ( other->m_Parent != this )
            {
                other->m_Children.emplace_back(this);
                m_Parent = other;
            }
            else
            {
                // TODO LOG -> cannot make circular dependency
            }
        }
    }

    void SceneComponent::detachSelf()
    {
        FV_CHECK_MO();
        detachParent();
        detachChildren();
    }

    void SceneComponent::detachParent()
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
    }

    void SceneComponent::serialize(TextSerializer& ts)
    {
        FV_CHECK_MO();
        u32 parentId = m_Parent ? ( m_Parent->gameObject() ? m_Parent->gameObject()->id() : -1 ) : -1;
        ts.serialize( "parent", parentId );
        ts.serialize( "position", m_Position );
        ts.serialize( "quat", m_Rotation );
        ts.serialize( "scale", m_Scale );
        ts.serialize( "sceneBits", m_SceneBits );
        // TODO
        if ( ts.isWriting() )
        {
        }
    }

    u64 SceneComponent::sceneBits() const
    {
        FV_CHECK_MO();
        return m_SceneBits;
    }

    bool SceneComponent::computeLocalToWorld()
    {
        FV_CHECK_MO();
        if ( m_MatrixDirty || (m_Parent && m_Parent->m_MatrixDirty) )
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
            m_MatrixDirty = false;
            return true; // Was dirty
        }
        return false; // Was not dirty
    }

    void SceneComponent::computeWorldToLocal()
    {
        FV_CHECK_MO();
        if ( computeLocalToWorld() ) // Update if localToWorld was dirty
        {
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
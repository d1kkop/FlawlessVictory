#pragma once
#include "GameComponent.h"
#include "../Core/Math.h"
#include "../Core/Reflection.h"
#include "../Core/GameObject.h"

namespace fv
{
    class SceneWorld;
    class TextSerializer;

    class SceneComponent: public GameComponent
    {
        FV_TYPE(SceneComponent)

        FV_MO FV_DLL SceneComponent();
        FV_MO FV_DLL ~SceneComponent() override;

    public:
        FV_DLL void move(const Vec3& translate);
        FV_DLL void setPosition(const Vec3& position);
        FV_DLL const Vec3& position() const;
        FV_DLL void rotate(const Quat& rotate);
        FV_DLL void setRotation(const Quat& rotation);
        FV_DLL const Quat& rotation() const;
        FV_DLL void scale(const Vec3& scale);
        FV_DLL void setScale(const Vec3& scale);
        FV_DLL const Vec3& scale() const;

        /*  Detaches from parent and attaches to new component if any. 
            World position/rotation/scale is not affected. */
        FV_MO FV_DLL bool attach(SceneComponent* other);
        FV_MO FV_DLL void detachFromParent();
        FV_MO FV_DLL void detachChildren();

        /*  Returns true of 'other' is in hierarchy of calling component.
            This can be either upwards (parent/ancestor) or downwards (child or grandchild). */
        FV_DLL bool inHierarchy(const SceneComponent* other) const;

        FV_DLL bool isChildOrGrandChild(const SceneComponent* other) const;

        FV_MO FV_DLL void serialize(TextSerializer& ts) override;
        FV_MO FV_DLL u64& sceneBits() { return m_SceneBits; }

        FV_DLL Mat4& localToWorld() { return m_GameObject->localToWorld(); }
        FV_DLL Mat4& worldToLocal() { return m_GameObject->worldToLocal(); }

    private:
        FV_MO void computeLocalToWorld();
        FV_MO void computeWorldToLocal();
        FV_MO void computeTRSFromWorldToLocal();

        Vec3 m_Position{};
        Quat m_Rotation = Quat::identity();
        Vec3 m_Scale{ 1,1,1 };
        SceneComponent* m_Parent{};
        Vector<SceneComponent*> m_Children;
        bool m_LocalToWorldDirty = true;
        u64 m_SceneBits = 0;

        friend class SceneManager;
    };

}
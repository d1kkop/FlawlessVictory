#pragma once
#include "../Core/Component.h"
#include "../Core/TypeManager.h"
#include "../Core/Math.h"

namespace fv
{
    class SceneComponent: public Component
    {
        FV_TYPE(SceneComponent)

    public:
        void move(const Vec3& translate);
        void setPosition(const Vec3& position);
        const Vec3& position() const;
        void rotate(const Quat& rotate);
        void setRotation(const Quat& rotation);
        const Quat& rotation() const;
        void scale(const Vec3& scale);
        void setScale(const Vec3& scale);
        const Vec3& scale() const;

        void attach(SceneComponent* other);
        void detachSelf();
        void detachParent();
        void detachChildren();
        

    private:
        bool computeLocalToWorld();
        void computeWorldToLocal();
        void computeTRSFromLocalToWorld();

        Vec3 m_Position{};
        Quat m_Rotation = Quat::identity();
        Vec3 m_Scale{ 1,1,1 };
        SceneComponent* m_Parent{};
        Array<SceneComponent*> m_Children;
        Mat4 m_LocalToWorld = Mat4::identity();
        Mat4 m_WorldToLocal = Mat4::identity();
        bool m_MatrixDirty{};
        bool m_WorldToLocalDirty{};
    };
}
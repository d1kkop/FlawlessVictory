#pragma once
#include "GameComponent.h"
#include "../Core/TypeManager.h"
#include "../Core/Math.h"
#include "../Core/Reflection.h"
#define FV_SCENECOMPONENT_PRIORITY (-1000)

namespace fv
{
    class SceneWorld;

    class SceneComponent: public GameComponent
    {
        FV_TYPE(SceneComponent)

        SceneComponent()
        {
            m_UpdatePriority = FV_SCENECOMPONENT_PRIORITY;
        }

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

        FV_MO FV_DLL void attach(SceneComponent* other);
        FV_MO FV_DLL void detachSelf();
        FV_MO FV_DLL void detachParent();
        FV_MO FV_DLL void detachChildren();

    private:
        FV_MO bool computeLocalToWorld();
        FV_MO void computeWorldToLocal();
        FV_MO void computeTRSWorldToLocal();

        Vec3 m_Position{};
        Quat m_Rotation = Quat::identity();
        Vec3 m_Scale{ 1,1,1 };
        SceneComponent* m_Parent{};
        Vector<SceneComponent*> m_Children;
        Mat4 m_LocalToWorld = Mat4::identity();
        Mat4 m_WorldToLocal = Mat4::identity();
        bool m_MatrixDirty{};
        bool m_WorldToLocalDirty{};
    };

}
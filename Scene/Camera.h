#pragma once
#include "SceneComponent.h"

namespace fv
{
    class Camera: public SceneComponent
    {
    public:
        FV_TYPE(Camera, FV_CAMERA_PRIORITY, false)

    public:
        void setFov(float degrees);
        void setAspect(float aspect);
        void setNear(float near);
        void setFar(float far);
        void setWidth(float width);
        void setHeight(float height);
        void setPerspective(bool isPerspective);
        Mat4 projection() const;

    private:
        float m_Fov = 90;
        float m_Aspect = 1.6f;
        float m_Near = 0.1f;
        float m_Far  = 1000.f;
        // In case of Ortho
        float m_Width  = 2.f*m_Aspect;
        float m_Height = 2.f;
        bool m_IsPerspective = true;
        mutable bool m_IsDirty = true;
        mutable Mat4 m_ProjectionMatrix = Mat4::identity();
    };
}
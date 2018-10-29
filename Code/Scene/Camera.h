#pragma once
#include "GameComponent.h"
#include "../Core/Math.h"
#include "../Core/Reflection.h"
#define FV_CAMERA_PRIORITY (9999)

namespace fv
{
    class TextSerializer;

    class Camera: public GameComponent
    {
        FV_TYPE(Camera)

    public:
        Camera() 
        {
            m_UpdatePriority = FV_CAMERA_PRIORITY;
        }

        void setFov(float degrees);
        void setAspect(float aspect);
        void setNear(float near);
        void setFar(float far);
        void setWidth(float width);
        void setHeight(float height);
        void setPerspective(bool isPerspective);
        Mat4 projection() const;

    protected:
        FV_MO FV_DLL void serialize( TextSerializer& ts ) override;

    private:
        float m_Fov = 90;
        float m_Aspect = 1.6f;
        float m_Near = 0.1f;
        float m_Far  = 1000.f;
        // In case of Ortho
        float m_Width  = 2.f*m_Aspect;
        float m_Height = 2.f;
        bool m_IsPerspective = true;
        bool m_IsStereo = false;
        float m_Ipd = 0.667f;
        mutable bool m_IsDirty = true;
        mutable Mat4 m_ProjectionMatrix = Mat4::identity();
    };
}
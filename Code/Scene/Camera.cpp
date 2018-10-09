#include "Camera.h"
#include "../Core/LogManager.h"

namespace fv
{
    FV_TYPE_IMPL(Camera)

    void Camera::setFov(float degrees)
    {
        if ( degrees < 0.1f )
        {
            LOGW("Camera FOV smaller than 0.1 degrees.");
        }
        m_Fov = degrees;
        m_IsDirty = true;
    }

    void Camera::setAspect(float aspect)
    {
        if ( aspect < 0.01f )
        {
            LOGW("Camera ASPECT smaller than 0.01.");
        }
        m_Aspect = aspect;
        m_IsDirty = true;
    }

    void Camera::setNear(float nearr)
    {
        if ( nearr < 0.01f )
        {
            LOGW("Camera NEAR smaller than 0.01.");
        }
        m_Near = nearr;
        m_IsDirty = true;
    }

    void Camera::setFar(float farr)
    {
        if ( farr > 10000.f )
        {
            LOGW("Camera FAR bigger than 10.000.");
        }
        m_Far = farr;
        m_IsDirty = true;
    }

    void Camera::setWidth(float width)
    {
        if ( width < 0.01f )
        {
            LOGW("Camera WIDTH smaller than 0.01.");
        }
        m_Width = width;
        m_IsDirty = true;
    }

    void Camera::setHeight(float height)
    {
        if ( height < 0.01f )
        {
            LOGW("Camera HEIGHT smaller than 0.01.");
        }
        m_Height = height;
        m_IsDirty = true;
    }

    void Camera::setPerspective(bool isPerspective)
    {
        m_IsPerspective = isPerspective;
        m_IsDirty = true;
    }

    Mat4 Camera::projection() const
    {
        if ( m_IsDirty )
        {
            if ( m_IsPerspective )
            {
                m_ProjectionMatrix = Mat4::perspectiveLH(D2R*m_Fov, m_Aspect, m_Near, m_Far);
            }
            else
            {
                m_ProjectionMatrix = Mat4::orthoLH(-m_Width*.5f, m_Width*.5f, -m_Height*.5f, m_Height*.5f, m_Near, m_Far);
            }
            m_IsDirty = false;
        }
        return m_ProjectionMatrix;
    }

}
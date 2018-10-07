#include "PCH.h"

namespace fv
{
    Quat Quat::identity()
    {
        return { 0, 0, 0, 1 };
    }

    Quat Quat::rotateX(float ang)
    {
        Quat q;
        q.setRotateX(ang);
        return q;
    }

    Quat Quat::rotateY(float ang)
    {
        Quat q;
        q.setRotateY(ang);
        return q;
    }

    Quat Quat::rotateZ(float ang)
    {
        Quat q;
        q.setRotateZ(ang);
        return q;
    }

    Quat Quat::rotate(const Vec3& axis, float ang)
    {
        Quat q;
        q.setRotate(axis, ang);
        return q;
    }

    Quat Quat::lookAt(const Vec3& forward, const Vec3& up)
    {
        Quat q;
        q.setLookAt(forward, up);
        return q;
    }

    Quat Quat::operator*(float f) const
    {
        return { x*f, y*f, z*f, w*f };
    }

    Quat Quat::operator*(const Quat& q) const
    {
        return cross(q);
    }

    Quat& Quat::operator*=(float f)
    {
        x*=f; y*=f; z*=f; w*=f;
        return *this;
    }

    Quat& Quat::operator*=(const Quat& q)
    {
       *this = cross(q);
       return *this;
    }

    Vec3 Quat::operator*(const Vec3& v) const
    {
        return transform(v);
    }

    float Quat::dot(const Quat& q) const
    {
        return x*q.x + y*q.y + z*q.z + w*q.w;
    }

    Quat Quat::cross(const Quat& q) const
    {
        return
        {
            w*q.x + x*q.w + z*q.y - y*q.z,
            w*q.y + y*q.w + x*q.z - z*q.x,
            w*q.z + z*q.w + y*q.x - x*q.y,
            w*q.w - x*q.x - y*q.y - z*q.z
        };
    }

    Quat Quat::normalized() const
    {
        Quat q = *this;
        q.normalize();
        return q;
    }

    bool Quat::isNormalized() const
    {
        return fabsf(dot(*this)-1.f)<0.0001f;
    }

    Quat Quat::conjugate() const
    {
        return { -x, -y, -z, w };
    }

    Quat Quat::inverse() const
    {
        if ( isNormalized() ) return conjugate();
        return conjugate().normalized();
    }

    float Quat::angle() const
    {
        float t=w;
        if ( t < -1.f ) t = -1.f;
        if ( t > 1.f )  t = 1.f;
        return acosf(t)*2.f;
    }

    Vec3 Quat::axis() const
    {
        float s = 1.f/(1.f-w*w);
        return { x*s, y*s, z*s };
    }

    Quat Quat::slerp(const Quat& q, float t) const
    {
        if ( t <= 0.f ) return *this;
        if ( t >= 1.f ) return q;

        float cso = dot(q);
        float x2=q.x, y2=q.y, z2=q.z, w2=q.w;
        if ( cso<0 )
        {
            x2=-x2, y2=-y2, z2=-z2, w2=-w2;
            cso=-cso;
        }
        assert(cso<1.001f);
        float s = 1.f-t;
        if ( cso<=0.9999f )
        {
            float so = sqrtf(1-cso*cso);
            float om = atan2f(so, cso);
            so = 1.f/so;
            s = sinf(s * om) * so;
            t = sinf(t * om) * so;
        }
        return
        {
            x*s + x2*t,
            y*s + y2*t,
            z*s + z2*t,
            w*s + w2*t
        };
    }

    Vec3 Quat::transform(const Vec3& v) const
    {
        // https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
        Vec3 t = 2.f * fv::cross({ x,y,z }, v);
        return v + w*t + fv::cross({ x,y,z }, t);
    }

    bool Quat::approx(const Quat& q, float eps) const
    {
        return dot(q)<eps;
    }

    Quat& Quat::normalize()
    {
        float m = 1.f/sqrt(dot(*this));
        x *= m;
        y *= m;
        z *= m;
        w *= m;
        assert( checkQuat(*this) );
        return *this;
    }

    Quat& Quat::setRotateX(float ang)
    {
        ang *= .5f;
        w = cosf(ang);
        x = sinf(ang);
        y = 0.f;
        z = 0.f;
        return *this;
    }

    Quat& Quat::setRotateY(float ang)
    {
        ang *= .5f;
        w = cosf(ang);
        x = 0.f;
        y = sinf(ang);
        z = 0.f;
        return *this;
    }

    Quat& Quat::setRotateZ(float ang)
    {
        ang *= .5f;
        w = cosf(ang);
        x = 0.f;
        y = 0.f;
        z = sinf(ang);
        return *this;
    }

    Quat& Quat::setRotate(const Vec3& axis, float ang)
    {
        assert(axis.isNormalized());
        ang *= .5f;
        float s = sinf(ang);
        w = cosf(ang);
        x = axis.x * s;
        y = axis.y * s;
        z = axis.z * s;
        return *this;
    }

    Quat& Quat::setLookAt(const Vec3& from, const Vec3& to)
    {
        Vec3 forward = (to - from).normalized();
        float fdot   = Vec3::forward() | forward;
        if ( fabsf(fdot - (-1.0f)) < 0.0001f )
        {
            // Point in opposite directions
            *this = Quat::angleAxis( Vec3::up(), PI );
        }
        else if ( fabsf(fdot - (1.0f)) < 0.0001f )
        {
            // Point in same direction
            *this = identity();
        }
        else
        {
            if ( fdot < -1.f ) fdot = -1.f;
            if ( fdot > 1.f )  fdot = 1.f;
            float ang = (float)acosf(fdot);
            Vec3 ax   = !(Vec3::forward() ^ forward);
            *this = Quat::angleAxis(ax, ang);
        }
        return *this;
    }

    Quat::operator Mat3() const
    {
        const float wx2 = w*x*2;
        const float wy2 = w*y*2;
        const float wz2 = w*z*2;
        const float xy2 = x*y*2;
        const float zz2 = z*z*2;
        const float yz2 = y*z*2;
        const float xx2 = x*x*2;
        const float yy2 = y*y*2;
        const float xz2 = x*z*2;
        return
        {
            1-yy2-zz2, xy2+wz2, xz2-wy2,
            xy2-wz2, 1-xx2-zz2, yz2+wx2,
            xz2+wy2, yz2-wx2, 1-xx2-yy2
        };
    }

    Quat::operator Mat4() const
    {
        const float wx2 = w*x*2;
        const float wy2 = w*y*2;
        const float wz2 = w*z*2;
        const float xy2 = x*y*2;
        const float zz2 = z*z*2;
        const float yz2 = y*z*2;
        const float xx2 = x*x*2;
        const float yy2 = y*y*2;
        const float xz2 = x*z*2;
        return
        {
            1-yy2-zz2, xy2+wz2, xz2-wy2, 0,
            xy2-wz2, 1-xx2-zz2, yz2+wx2, 0,
            xz2+wy2, yz2-wx2, 1-xx2-yy2, 0,
            0, 0, 0, 1
        };
    }
}
#include "Vec3.h"

namespace fv
{
    Vec3 Vec3::cross(const Vec3& o) const
    {
        return
        {
            y*o.z - o.y*z,
            z*o.x - o.z*x,
            x*o.y - o.x*y
        };
    }

    Vec3 Vec3::normalized() const
    {
        Vec3 c = *this;
        return c.normalize();
    }

    bool Vec3::isNormalized() const
    {
        return fabsf(dot(*this)-1)<0.0001f;
    }

    Vec3 Vec3::lerp(const Vec3& to, float t) const
    {
        return { x+(to.x-x)*t, y+(to.y-y)*t, z+(to.z-z)*t };
    }

    bool Vec3::approx(const Vec3& o, float eps) const
    {
        Vec3 d = o-*this;
        return fabsf(d.x)<eps && fabsf(d.y)<eps && fabsf(d.z)<eps;
    }

    Vec3& Vec3::normalize()
    {
        float l=1.f/length();
        *this *= l;
        // TODO check nan
        return *this;
    }

    Vec3& Vec3::clamp(float low, float high)
    {
        if ( x < low ) x = low;
        else if ( x > high ) x = high;
        if ( y < low ) y = low;
        else if ( y > high ) y = high;
        if ( z < low ) z = low;
        else if ( z > high ) z = high;
        return *this;
    }
}
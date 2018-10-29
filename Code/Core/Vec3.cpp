#include "PCH.h"
#include "Functions.h"

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
        float l=length();
        if (l>=0.00001f)
            l=1.f/l;
        *this *= l;
        assert( checkVector(*this) );
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

    Vec3& Vec3::min(const Vec3& o)
    {
        x = Min(x, o.x);
        y = Min(y, o.y);
        z = Min(z, o.z);
        return *this;
    }

    Vec3& Vec3::max(const Vec3& o)
    {
        x = Max(x, o.x);
        y = Max(y, o.y);
        z = Max(z, o.z);
        return *this;
    }

}
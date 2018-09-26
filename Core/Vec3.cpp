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

    Vec3& Vec3::normalize()
    {
        float l=1.f/length();
        *this *= l;
        // TODO check nan
        return *this;
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
}
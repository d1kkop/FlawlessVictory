#include "Vec4.h"
#include "Vec3.h"
#include <cmath>

namespace fv
{
    Vec4 Vec4::normalized() const
    {
        Vec4 c = *this;
        return c.normalize();
    }

    bool Vec4::isNormalized() const
    {
        return fabsf(dot(*this)-1)<0.0001f;
    }

    Vec4& Vec4::normalize()
    {
        float l=1.f/length();
        *this *= l;
        // TODO check nan
        return *this;
    }

    Vec4::operator Vec3() const
    {
        return { x, y, z };
    }
}
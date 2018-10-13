#include "PCH.h"

namespace fv
{
    Vec2 Vec2::normalized() const
    {
        Vec2 c = *this;
        return c.normalize();
    }

    bool Vec2::isNormalized() const
    {
        return fabsf(dot(*this)-1)<0.0001f;
    }

    Vec2 Vec2::lerp(const Vec2& to, float t) const
    {
        return { x+(to.x-x)*t, y+(to.y-y)*t };
    }

    bool Vec2::approx(const Vec2& o, float eps) const
    {
        Vec2 d = o-*this;
        return fabsf(d.x)<eps && fabsf(d.y)<eps;
    }

    Vec2& Vec2::normalize()
    {
        float l=1.f/length();
        *this *= l;
        assert( checkVector(*this) );
        return *this;
    }

    Vec2& Vec2::clamp(float low, float high)
    {
        if ( x < low ) x = low;
        else if ( x > high ) x = high;
        if ( y < low ) y = low;
        else if ( y > high ) y = high;
        return *this;
    }
}
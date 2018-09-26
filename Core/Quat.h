#pragma once
#include "Platform.h"
#include "Math.h"

namespace fv
{
    struct Vec3;
    struct Mat3;

    struct FV_DLL Quat
    {
        union
        {
            struct { float x, y, z, w; };
            float m[4];
        };

        static Quat identity();
        static Quat rotateX(float ang);
        static Quat rotateY(float ang);
        static Quat rotateZ(float ang);
        static Quat rotate(const Vec3& axis, float ang);

        Quat operator* (float f) const;
        Quat operator* (const Quat& q) const;
        Quat& operator*= (float f);
        Quat& operator*= (const Quat& q);

        float dot(const Quat& q) const;
        Quat  cross(const Quat& q) const;
        Quat  normalized() const;
        bool  isNormalized() const;
        Quat  conjugate() const;
        Quat  inverse() const;
        float angle() const;
        Vec3  axis() const;
        Quat  slerp(const Quat& q, float t) const;
        Vec3  transform(const Vec3& v) const;

        Quat& normalize();
        Quat& setRotateX(float ang);
        Quat& setRotateY(float ang);
        Quat& setRotateZ(float ang);
        Quat& setRotate(const Vec3& axis, float ang);

        explicit operator Mat3 () const;
    };


    // Short normalize
    inline Quat operator! (const Quat& n) { return n.normalized(); }

    // Short dot
    inline float operator| (const Quat& a, const Quat& b) { return a.dot(b); }

    // Short cross
    inline Quat operator^ (const Quat& a, const Quat& b) { return a.cross(b); }

    // Global dot
    inline float dot(const Quat& a, const Quat& b) { return a|b; }

    // Global cross
    inline Quat cross(const Quat& a, const Quat& b) { return a^b; }

    // Global slerp
    inline Quat slerp(const Quat& a, const Quat& b, float t) { return a.slerp(b, t); }
}
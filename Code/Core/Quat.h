#pragma once
#include "Platform.h"

namespace fv
{
    struct Vec3;
    struct Mat3;
    struct Mat4;

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
        static Quat lookAt(const Vec3& from, const Vec3& to);
        static Quat angleAxis(const Vec3& axis, float ang) { return Quat::rotate(axis, ang); }

        Quat operator* (float f) const;
        Quat operator* (const Quat& q) const;
        Vec3 operator* (const Vec3& v) const;
        Quat& operator*= (float f);
        Quat& operator*= (const Quat& q);
        bool operator== (const Quat& q) const { return x==q.x && y==q.y && z==q.z && w==q.w; }
        bool operator!= (const Quat& q) const { return !(*this==q); }

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
        bool  approx(const Quat& q, float eps=0.0001f) const;

        Quat& normalize();
        Quat& setRotateX(float ang);
        Quat& setRotateY(float ang);
        Quat& setRotateZ(float ang);
        Quat& setRotate(const Vec3& axis, float ang);
        Quat& setLookAt(const Vec3& from, const Vec3& to);

        explicit operator Mat3 () const;
        explicit operator Mat4 () const;
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
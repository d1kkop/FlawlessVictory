#pragma once
#include "Platform.h"
#include "Math.h"

namespace fv
{
    struct Vec4;

    struct FV_DLL Vec3
    {
        union
        {
            struct { float x, y, z; };
            float m[3];
        };

        Vec3 operator + (float f) const { return { x+f, y+f, z+f }; }
        Vec3 operator - (float f) const { return { x-f, y-f, z-f }; }
        Vec3 operator * (float f) const { return { x*f, y*f, z*f }; }
        Vec3 operator / (float f) const { return { x/f, y/f, z/f }; }

        Vec3 operator + (const Vec3& v) const { return { x+v.x, y+v.y, z+v.z }; }
        Vec3 operator - (const Vec3& v) const { return { x-v.x, y-v.y, z-v.z }; }
        Vec3 operator * (const Vec3& v) const { return { x*v.x, y*v.y, z*v.z }; }
        Vec3 operator / (const Vec3& v) const { return { x/v.x, y/v.y, z/v.z }; }

        Vec3& operator += (float f) { x += f; y += f; z += f; return *this; }
        Vec3& operator -= (float f) { x -= f; y -= f; z -= f; return *this; }
        Vec3& operator *= (float f) { x *= f; y *= f; z *= f; return *this; }
        Vec3& operator /= (float f) { x /= f; y /= f; z /= f; return *this; }

        Vec3& operator += (const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
        Vec3& operator -= (const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
        Vec3& operator *= (const Vec3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
        Vec3& operator /= (const Vec3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }

        float length()                      const { return sqrtf(lengthSq()); }
        float lengthSq()                    const { return x*x + y*y + z*z; }
        float dist(const Vec3& o)           const { return (o-*this).length(); }
        float distSq(const Vec3& o)         const { return (o-*this).lengthSq(); }
        float dot(const Vec3& o)            const { return x*o.x + y*o.y + z*o.z; }

        Vec3 cross(const Vec3& o) const;
        Vec3 normalized() const;
        bool isNormalized() const;
        Vec3& normalize();
    };

    // Short normalize
    inline Vec3 operator! (const Vec3& n) { return n.normalized(); }

    // Short dot
    inline float operator| (const Vec3& a, const Vec3& b) { return a.dot(b); }

    // Short cross
    inline Vec3 operator^ (const Vec3& a, const Vec3& b) { return a.cross(b); }

    // Global dot
    inline float dot(const Vec3& a, const Vec3& b) { return a|b; }

    // Global cross
    inline Vec3 cross(const Vec3& a, const Vec3& b) { return a^b; }

    // Oposite scalar operations
    inline Vec3 operator+ (float f, const Vec3& v) { return v+f; }
    inline Vec3 operator- (float f, const Vec3& v) { return v-f; }
    inline Vec3 operator* (float f, const Vec3& v) { return v*f; }
    inline Vec3 operator/ (float f, const Vec3& v) { return v/f; }
}
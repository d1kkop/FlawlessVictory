#pragma once
#include "Platform.h"
#include <cmath>

namespace fv
{
    struct Vec3;

    struct FV_DLL Vec4
    {
        union
        {
            struct { float x, y, z, w; };
            float m[4];
        };

        Vec4 operator -() const { return { -x, -y, -z, -w }; }

        Vec4 operator + (float f) const { return { x+f, y+f, z+f, w+f }; }
        Vec4 operator - (float f) const { return { x-f, y-f, z-f, w-f }; }
        Vec4 operator * (float f) const { return { x*f, y*f, z*f, w*f }; }
        Vec4 operator / (float f) const { return { x/f, y/f, z/f, w/f }; }
        
        Vec4 operator + (const Vec4& v) const { return { x+v.x, y+v.y, z+v.z, w+v.w }; }
        Vec4 operator - (const Vec4& v) const { return { x-v.x, y-v.y, z-v.z, w-v.w }; }
        Vec4 operator * (const Vec4& v) const { return { x*v.x, y*v.y, z*v.z, w*v.w }; }
        Vec4 operator / (const Vec4& v) const { return { x/v.x, y/v.y, z/v.z, w/v.w }; }
        
        Vec4& operator += (float f) { x += f; y += f; z += f; w += f; return *this; }
        Vec4& operator -= (float f) { x -= f; y -= f; z -= f; w -= f; return *this; }
        Vec4& operator *= (float f) { x *= f; y *= f; z *= f; w *= f; return *this; }
        Vec4& operator /= (float f) { x /= f; y /= f; z /= f; w /= f; return *this; }
        
        Vec4& operator += (const Vec4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
        Vec4& operator -= (const Vec4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
        Vec4& operator *= (const Vec4& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
        Vec4& operator /= (const Vec4& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }

        bool operator== (const Vec4& v) const { return x==v.x && y==v.y && z==v.z && w==v.w; }
        bool operator!= (const Vec4& v) const { return !(*this==v); }

        float length()                      const { return sqrtf(lengthSq()); }
        float lengthSq()                    const { return x*x + y*y + z*z + w*w; }
        float dist(const Vec4& o)           const { return (o-*this).length(); }
        float distSq(const Vec4& o)         const { return (o-*this).lengthSq(); }
        float dot(const Vec4& o)            const { return x*o.x + y*o.y + z*o.z + w*o.w; }

        Vec4 normalized() const;
        bool isNormalized() const;
        Vec4& normalize();
        bool approx(const Vec4& o, float eps=0.0001f) const;

        explicit operator Vec3 () const;
    };

    // Short normalize
    inline Vec4 operator! (const Vec4& n) { return n.normalized(); }

    // Short dot
    inline float operator| (const Vec4& a, const Vec4& b) { return a.dot(b); }

    // Global dot
    inline float dot(const Vec4& a, const Vec4& b) { return a|b; }

    // Oposite scalar operations
    inline Vec4 operator+ (float f, const Vec4& v) { return v+f; }
    inline Vec4 operator- (float f, const Vec4& v) { return v-f; }
    inline Vec4 operator* (float f, const Vec4& v) { return v*f; }
    inline Vec4 operator/ (float f, const Vec4& v) { return v/f; }
}
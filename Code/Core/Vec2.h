#pragma once
#include "Platform.h"
#include <cmath>

namespace fv
{
    struct FV_DLL Vec2
    {
        union
        {
            struct { float x, y; };
            float m[2];
        };

        static Vec2 left()      { return { -1, 0 }; }
        static Vec2 right()     { return { 1, 0 }; }
        static Vec2 up()        { return { 0, 1 }; }
        static Vec2 down()      { return { 0, -1 }; }
        static Vec2 zero()      { return { 0, 0 }; }
        static Vec2 one()       { return { 1, 1 }; }

        Vec2 operator -() const { return { -x, -y }; }
        
        Vec2 operator + (float f) const { return { x+f, y+f }; }
        Vec2 operator - (float f) const { return { x-f, y-f }; }
        Vec2 operator * (float f) const { return { x*f, y*f }; }
        Vec2 operator / (float f) const { return { x/f, y/f }; }
        
        Vec2 operator + (const Vec2& v) const { return { x+v.x, y+v.y }; }
        Vec2 operator - (const Vec2& v) const { return { x-v.x, y-v.y }; }
        Vec2 operator * (const Vec2& v) const { return { x*v.x, y*v.y }; }
        Vec2 operator / (const Vec2& v) const { return { x/v.x, y/v.y }; }
        
        Vec2& operator += (float f) { x += f; y += f; return *this; }
        Vec2& operator -= (float f) { x -= f; y -= f; return *this; }
        Vec2& operator *= (float f) { x *= f; y *= f; return *this; }
        Vec2& operator /= (float f) { x /= f; y /= f; return *this; }
        
        Vec2& operator += (const Vec2& v) { x += v.x; y += v.y; return *this; }
        Vec2& operator -= (const Vec2& v) { x -= v.x; y -= v.y; return *this; }
        Vec2& operator *= (const Vec2& v) { x *= v.x; y *= v.y; return *this; }
        Vec2& operator /= (const Vec2& v) { x /= v.x; y /= v.y; return *this; }

        bool operator== (const Vec2& v) const { return x==v.x && y==v.y; }
        bool operator!= (const Vec2& v) const { return !(*this==v); }

        float length()                      const { return sqrtf(lengthSq()); }
        float lengthSq()                    const { return x*x + y*y; }
        float dist(const Vec2& o)           const { return (o-*this).length(); }
        float distSq(const Vec2& o)         const { return (o-*this).lengthSq(); }
        float dot(const Vec2& o)            const { return x*o.x + y*o.y; }

        Vec2 normalized() const;
        bool isNormalized() const;
        Vec2 lerp(const Vec2& to, float t) const;
        bool approx(const Vec2& o, float eps=0.0001f) const;
        Vec2& normalize();
        Vec2& clamp(float low, float high);
    };

    // Short normalize
    inline Vec2 operator! (const Vec2& n) { return n.normalized(); }

    // Short dot
    inline float operator| (const Vec2& a, const Vec2& b) { return a.dot(b); }

    // Global dot
    inline float dot(const Vec2& a, const Vec2& b) { return a|b; }

    // Opposite scalar operations
    inline Vec2 operator+ (float f, const Vec2& v) { return v+f; }
    inline Vec2 operator* (float f, const Vec2& v) { return v*f; }
}
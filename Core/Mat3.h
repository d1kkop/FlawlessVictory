#pragma once
#include "Platform.h"

namespace fv
{
    struct Vec3;
    struct Quat;
    struct Mat4;

    struct FV_DLL Mat3
    {
        union
        {
            struct
            {
                float m11, m12, m13;
                float m21, m22, m23;
                float m31, m32, m33;
            };
            float cells[9];
        };

        static Mat3 identity();
        static Mat3 scale(const Vec3 s);
        static Mat3 rotateX(float ang);
        static Mat3 rotateY(float ang);
        static Mat3 rotateZ(float ang);
        static Mat3 rotate(const Vec3& axis, float ang);
        static Mat3 lookAt(const Vec3& from, const Vec3& to);
        static Mat3 angleAxis(const Vec3& axis, float ang) { return rotate(axis, ang); }
        static Mat3 fromAxis(const Vec3& x, const Vec3& y, const Vec3& z);

        Vec3 operator* (const Vec3& p) const;
        Mat3 operator* (const Mat3& m) const;
        bool operator== (const Mat3& o) const;
        bool operator!= (const Mat3& o) const { return !(*this==o); }

        float determinant() const;
        Mat3  multiply(const Mat3& m) const;
        Mat3  inverse() const;
        Mat3  transpose() const;
        Vec3  transform(const Vec3& p) const;
        Vec3  axisX() const;
        Vec3  axisY() const;
        Vec3  axisZ() const;

        Mat3& setScale(const Vec3& scale);
        Mat3& setRotate(const Vec3& axis, float ang);
        Mat3& setRotateX(float ang);
        Mat3& setRotateY(float ang);
        Mat3& setRotateZ(float ang);
        Mat3& setLookAt(const Vec3& forward, const Vec3& up);
        Mat3& setFromAxis(const Vec3& x, const Vec3& y, const Vec3& z);

        explicit operator Quat () const;
        explicit operator Mat4 () const;
    };
}
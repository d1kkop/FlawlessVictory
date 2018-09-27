#pragma once
#include "Platform.h"

namespace fv
{
    struct Vec3;
    struct Vec4;
    struct Mat3;
    struct Quat;

    struct FV_DLL Mat4
    {
        union
        {
            struct
            {
                float m11, m12, m13, m14;
                float m21, m22, m23, m24;
                float m31, m32, m33, m34;
                float m41, m42, m43, m44;
            };
            float cells[16];
        };

        static Mat4 identity();
        static Mat4 orthoLH(float left, float right, float bottom, float top, float znear, float zfar);
        static Mat4 orthoRH(float left, float right, float bottom, float top, float znear, float zfar);
        static Mat4 frustumLH(float left, float right, float bottom, float top, float znear, float zfar);
        static Mat4 frustumRH(float left, float right, float bottom, float top, float znear, float zfar);
        static Mat4 perspectiveLH(float fov, float aspect, float znear, float zfar);
        static Mat4 perspectiveRH(float fov, float aspect, float znear, float zfar);
        static Mat4 scale(const Vec3& s);
        static Mat4 translate(const Vec3& t);
        static Mat4 rotate(const Vec3& axis, float ang);
        static Mat4 rotateX(float ang);
        static Mat4 rotateY(float ang);
        static Mat4 rotateZ(float ang);
        static Mat4 compose(Vec3& p, Quat& q, Vec3& s);

        Vec3 operator* (const Vec3& p) const;
        Vec4 operator* (const Vec4& p) const;
        Mat4 operator* (const Mat4& m) const;
        bool operator== (const Mat4& o) const;
        bool operator!= (const Mat4& o) const { return !(*this==o); }

        Mat4 multiply(const Mat4& m) const;
        Mat4 inverse() const;
        Mat4 transpose() const;
        Vec3 transform(const Vec3& p) const;
        Vec4 transform(const Vec4& p) const;
        Vec4 axisX() const;
        Vec4 axisY() const;
        Vec4 axisZ() const;
        Vec4 translation() const;
        Quat rotation() const;
        Vec3 scale() const;
        void decompose(Vec3& p, Quat& q, Vec3& s) const;

        Mat4& setScale(const Vec3& s);
        Mat4& setTranslate(const Vec3& t);
        Mat4& setRotate(const Vec3& axis, float ang);
        Mat4& setRotateX(float ang);
        Mat4& setRotateY(float ang);
        Mat4& setRotateZ(float ang);

        explicit operator Quat () const;
        explicit operator Mat3 () const;
    };

}
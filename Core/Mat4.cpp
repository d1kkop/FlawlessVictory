#include "Mat4.h"
#include "Quat.h"
#include "Mat3.h"
#include "Vec3.h"
#include "Vec4.h"
#include <cassert>

namespace fv
{
    Mat4 Mat4::identity()
    {
        return
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
    }

    Mat4 Mat4::orthoRH(float left, float right, float bottom, float top, float znear, float zfar)
    {
        return
        {
            2.f/(right-left), 0, 0, -(right+left)/(right-left),
            0, 2.f/(top-bottom), 0, -(top+bottom)/(top-bottom),
            0, 0, -2.f/(zfar-znear), -(zfar+znear)/(zfar-znear),
            0, 0, 0, 0
        };
    }

    Mat4 Mat4::frustumRH(float left, float right, float bottom, float top, float znear, float zfar)
    {
        return
        {
            (2.f*znear)/(right-left), 0, (right+left)/(right-left), 0,
            0, (2.f*znear)/(top-bottom), (top+bottom)/(top-bottom), 0,
            0, 0, -(zfar+znear)/(zfar-znear), -(2.f*zfar*znear)/(zfar-znear),
            0, 0, -1, 0
        };
    }

    Mat4 Mat4::perspectiveRH(float fov, float aspect, float znear, float zfar)
    {
        float y = tanf(fov*.5f)*znear;
        float x = y*aspect;
        return frustumRH(-x, x, -y, y, znear, zfar);
    }

    Mat4 Mat4::compose(Vec3& p, Quat& q, Vec3& s)
    {
        Mat4 T = Mat4::translate(p);
        Mat4 R = Mat4::rotate(q.axis(), q.angle());
        Mat4 S = Mat4::scale(s);
        return (T*R*S);
    }

    Vec3 Mat4::operator*(const Vec3& p) const
    {
        return transform(p);
    }

    Vec4 Mat4::operator*(const Vec4& p) const
    {
        return transform(p);
    }

    Mat4 Mat4::operator*(const Mat4& m) const
    {
        return multiply(m);
    }

    Mat4 Mat4::multiply(const Mat4& m) const
    {
        return
        {
            m11*m.m11 + m12*m.m21 + m13*m.m31 + m14*m.m41,
            m11*m.m12 + m12*m.m22 + m13*m.m32 + m14*m.m42,
            m11*m.m13 + m12*m.m23 + m13*m.m33 + m14*m.m43,
            m11*m.m14 + m12*m.m24 + m13*m.m34 + m14*m.m44,

            m21*m.m11 + m22*m.m21 + m23*m.m31 + m24*m.m41,
            m21*m.m12 + m22*m.m22 + m23*m.m32 + m24*m.m42,
            m21*m.m13 + m22*m.m23 + m23*m.m33 + m24*m.m43,
            m21*m.m14 + m22*m.m24 + m23*m.m34 + m24*m.m44,

            m31*m.m11 + m32*m.m21 + m33*m.m31 + m34*m.m41,
            m31*m.m12 + m32*m.m22 + m33*m.m32 + m34*m.m42,
            m31*m.m13 + m32*m.m23 + m33*m.m33 + m34*m.m43,
            m31*m.m14 + m32*m.m24 + m33*m.m34 + m34*m.m44,

            m41*m.m11 + m42*m.m21 + m43*m.m31 + m44*m.m41,
            m41*m.m12 + m42*m.m22 + m43*m.m32 + m44*m.m42,
            m41*m.m13 + m42*m.m23 + m43*m.m33 + m44*m.m43,
            m41*m.m14 + m42*m.m24 + m43*m.m34 + m44*m.m44
        };
    }

    Mat4 Mat4::inverse() const
    {
        float s0 = m11*m22-m12*m21;
        float s1 = m11*m23-m13*m21;
        float s2 = m11*m24-m14*m21;
        float s3 = m12*m23-m13*m22;
        float s4 = m12*m24-m14*m22;
        float s5 = m13*m24-m14*m23;
        float c0 = m31*m42-m32*m41;
        float c1 = m31*m43-m33*m41;
        float c2 = m31*m44-m34*m41;
        float c3 = m32*m43-m33*m42;
        float c4 = m32*m44-m34*m42;
        float c5 = m33*m44-m34*m43;

        float det = 1.f / (s0*c5 - s1*c4 + s2*c3 + s3*c2 - s4*c1 + s5*c0);
        // TODO check nan

        return
        {
            (m22*c5 - m23*c4 + m24*c3) * det,
            (-m12*c5 + m13*c4 - m14*c3) * det,
            (m42*s5 - m43*s4 + m44*s3) * det,
            (-m32*s5 + m33*s4 - m34*s3) * det,

            (-m21*c5 + m23*c2 - m24*c1) * det,
            (m11*c5 - m13*c2 + m14*c1) * det,
            (-m41*s5 + m43*s2 - m44*s1) * det,
            (m31*s5 - m33*s2 + m34*s1) * det,

            (m21*c4 - m22*c2 + m24*c0) * det,
            (-m11*c4 + m12*c2 - m14*c0) * det,
            (m41*s4 - m42*s2 + m44*s0) * det,
            (-m31*s4 + m32*s2 - m34*s0) * det,

            (-m21*c3 + m22*c1 - m23*c0) * det,
            (m11*c3 - m12*c1 + m13*c0) * det,
            (-m41*s3 + m42*s1 - m43*s0) * det,
            (m31*s3 - m32*s1 + m33*s0) * det,
        };
    }

    Mat4 Mat4::transpose() const
    {
        return
        {
            m11, m21, m31, m41,
            m12, m22, m32, m42,
            m13, m23, m33, m43,
            m14, m24, m34, m44
        };
    }

    Vec3 Mat4::transform(const Vec3& p) const
    {
        return
        {
            p.x*m11 + p.y*m21 + p.z*m31 + m41,
            p.x*m12 + p.y*m22 + p.z*m32 + m42,
            p.x*m13 + p.y*m23 + p.z*m33 + m43
        };
    }

    Vec4 Mat4::transform(const Vec4& p) const
    {
        return
        {
            p.x*m11 + p.y*m21 + p.z*m31 + p.w*m41,
            p.x*m12 + p.y*m22 + p.z*m32 + p.w*m42,
            p.x*m13 + p.y*m23 + p.z*m33 + p.w*m43,
            p.x*m14 + p.y*m24 + p.z*m34 + p.w*m44
        };
    }

    Vec4 Mat4::axisX() const
    {
        return { m11, m12, m13, m14 };
    }

    Vec4 Mat4::axisY() const
    {
        return { m21, m22, m23, m24 };
    }

    Vec4 Mat4::axisZ() const
    {
        return { m31, m32, m33, m34 };
    }

    Vec4 Mat4::translation() const
    {
        return { m41, m42, m43, m44 };
    }

    Quat Mat4::rotation() const
    {
        return (Quat)*this;
    }

    Vec3 Mat4::scale() const
    {
        return { axisX().length(), axisY().length(), axisZ().length() };
    }

    Mat4 Mat4::scale(const Vec3& s)
    {
        Mat4 m;
        m.setScale(s);
        return m;
    }

    Mat4 Mat4::translate(const Vec3& t)
    {
        Mat4 m;
        m.setTranslate(t);
        return m;
    }

    Mat4 Mat4::rotate(const Vec3& axis, float ang)
    {
        Mat4 m;
        m.setRotate(axis, ang);
        return m;
    }

    Mat4 Mat4::rotateX(float ang)
    {
        Mat4 m;
        m.setRotateX(ang);
        return m;
    }

    Mat4 Mat4::rotateY(float ang)
    {
        Mat4 m;
        m.setRotateY(ang);
        return m;
    }

    Mat4 Mat4::rotateZ(float ang)
    {
        Mat4 m;
        m.setRotateZ(ang);
        return m;
    }

    void Mat4::decompose(Vec3& p, Quat& q, Vec3& s) const
    {
        p = (Vec3)translation();
        q = rotation();
        s = scale();
    }

    Mat4& Mat4::setScale(const Vec3& s)
    {
        *this = identity();
        m11=s.x, m22=s.y, m33=s.z;
        return *this;
    }

    Mat4& Mat4::setTranslate(const Vec3& t)
    {
        *this = identity();
        m41=t.x, m42=t.y, m43=t.z;
        return *this;
    }

    Mat4& Mat4::setRotate(const Vec3& axis, float ang)
    {
        assert(axis.isNormalized());
        float s = sinf(ang);
        float c = cosf(ang);
        float a = 1-c;
        float ax=a*axis.x;
        float ay=a*axis.y;
        float az=a*axis.z;
        float xs=axis.x*s;
        float ys=axis.y*s;
        float zs=axis.z*s;
        m11=ax*axis.x+c;
        m12=ax*axis.y+zs;
        m13=ax*axis.z-ys;
        m21=ay*axis.x-zs;
        m22=ay*axis.y+c;
        m23=ay*axis.z+xs;
        m31=az*axis.x+ys;
        m32=az*axis.y-xs;
        m33=az*axis.z+c;
        return *this;
        {
            m14=0, m24=0, m34=0;
            m41=0, m42=0, m43=0;
            m44=1;
        }
        return *this;
    }

    Mat4& Mat4::setRotateX(float ang)
    {
        float s = sinf(ang);
        float c = cosf(ang);
        m11=1, m12=0, m13=0;
        m21=0, m22=c, m23=s;
        m31=0, m32=-s, m33=c;
        {
            m14=0, m24=0, m34=0;
            m41=0, m42=0, m43=0;
            m44=1;
        }
        return *this;
    }

    Mat4& Mat4::setRotateY(float ang)
    {
        float s = sinf(ang);
        float c = cosf(ang);
        m11=c, m12=0, m13=-s;
        m21=0, m22=1, m23=0;
        m31=s, m32=0, m33=c;
        {
            m14=0, m24=0, m34=0;
            m41=0, m42=0, m43=0;
            m44=1;
        }
        return *this;
    }

    Mat4& Mat4::setRotateZ(float ang)
    {
        float s = sinf(ang);
        float c = cosf(ang);
        m11=c, m12=s, m13=0;
        m21=-s, m22=c, m23=0;
        m31=0, m32=0, m33=1;
        {
            m14=0, m24=0, m34=0;
            m41=0, m42=0, m43=0;
            m44=1;
        }
        return *this;
    }

    Mat4::operator Quat() const
    {
        return (Quat)((Mat3)*this);
    }

    Mat4::operator Mat3 () const
    {
        return 
        {
            m11, m12, m13,
            m21, m22, m23,
            m31, m32, m33
        };
    }

}
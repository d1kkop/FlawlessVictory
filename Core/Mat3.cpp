#include "Mat3.h"
#include "Quat.h"
#include <cassert>
#include <cmath>

namespace fv
{
    Mat3 Mat3::identity()
    {
        return 
        {
            1, 0, 0,
            0, 1, 0,
            0, 0, 1
        };
    };

    Mat3 Mat3::scale(const Vec3 s)
    {
        Mat3 m; 
        m.setScale(s); 
        return m; 
    }

    Mat3 Mat3::rotateX(float ang)
    {
        Mat3 m;
        m.setRotateX(ang); 
        return m;
    }

    Mat3 Mat3::rotateY(float ang)
    {
        Mat3 m; 
        m.setRotateY(ang); 
        return m;
    }

    Mat3 Mat3::rotateZ(float ang)
    {
        Mat3 m; 
        m.setRotateZ(ang); 
        return m;
    }

    Mat3 Mat3::rotate(const Vec3& axis, float ang)
    {
        Mat3 m; 
        m.setRotate(axis, ang); 
        return m;
    }

    Mat3 Mat3::lookAt(const Vec3& from, const Vec3& to)
    {
        Mat3 m;
        m.setLookAt(from, to);
        return m;
    }

    Mat3 Mat3::fromAxis(const Vec3& x, const Vec3& y, const Vec3& z)
    {
        Mat3 m;
        m.setFromAxis(x,y,z);
        return m;
    }

    Vec3 Mat3::operator*(const Vec3& p) const
    {
        return transform(p);
    }

    Mat3 Mat3::operator*(const Mat3& m) const
    {
        return multiply(m);
    }

    bool Mat3::operator==(const Mat3& o) const
    {
        return memcmp(this, &o, sizeof(*this))==0;
    }

    float Mat3::determinant() const
    {
        return
            m11 * (m22*m33 - m23*m32) +
            m12 * (m23*m31 - m21*m33) +
            m13 * (m21*m32 - m22*m31);
    }

    Mat3 Mat3::multiply(const Mat3& m) const
    {
        return
        {
            m11*m.m11 + m12*m.m21 + m13*m.m31,
            m11*m.m12 + m12*m.m22 + m13*m.m32,
            m11*m.m13 + m12*m.m23 + m13*m.m33,
            m21*m.m11 + m22*m.m21 + m23*m.m31,
            m21*m.m12 + m22*m.m22 + m23*m.m32,
            m21*m.m13 + m22*m.m23 + m23*m.m33,
            m31*m.m11 + m32*m.m21 + m33*m.m31,
            m31*m.m12 + m32*m.m22 + m33*m.m32,
            m31*m.m13 + m32*m.m23 + m33*m.m33
        };
    }

    Mat3 Mat3::inverse() const
    {
        float det = 1.f/determinant();
        // TODO check nan
        return
        {
            (m22*m33 - m23*m32) * det,
            (m13*m32 - m12*m33) * det,
            (m12*m23 - m13*m22) * det,
            (m23*m31 - m21*m33) * det,
            (m11*m33 - m13*m31) * det,
            (m13*m21 - m11*m23) * det,
            (m21*m32 - m22*m31) * det,
            (m12*m31 - m11*m32) * det,
            (m11*m22 - m12*m21) * det,
        };
    }

    Mat3 Mat3::transpose() const
    {
        return
        {
            m11, m21, m31,
            m12, m22, m32,
            m13, m23, m33
        };
    }

    Vec3 Mat3::transform(const Vec3& p) const
    {
        return
        {
            p.x*m11 + p.y*m21 + p.z*m31,
            p.x*m12 + p.y*m22 + p.z*m32,
            p.x*m13 + p.y*m23 + p.z*m33
        };
    }

    Vec3 Mat3::axisX() const
    {
        return { m11, m12, m13 };
    }

    Vec3 Mat3::axisY() const
    {
        return { m21, m22, m23 };
    }

    Vec3 Mat3::axisZ() const
    {
        return { m31, m32, m33 };
    }

    Mat3& Mat3::setScale(const Vec3& s)
    {
        m11=s.x, m12=0, m13=0;
        m21=0, m22=s.y, m23=0;
        m31=0, m32=0, m33=s.z;
        return *this;
    }

    Mat3& Mat3::setRotate(const Vec3& axis, float ang)
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
    }

    Mat3& Mat3::setRotateX(float ang)
    {
        float s = sinf(ang);
        float c = cosf(ang);
        m11=1, m12=0, m13=0;
        m21=0, m22=c, m23=s;
        m31=0, m32=-s, m33=c;
        return *this;
    }

    Mat3& Mat3::setRotateY(float ang)
    {
        float s = sinf(ang);
        float c = cosf(ang);
        m11=c, m12=0, m13=-s;
        m21=0, m22=1, m23=0;
        m31=s, m32=0, m33=c;
        return *this;
    }

    Mat3& Mat3::setRotateZ(float ang)
    {
        float s = sinf(ang);
        float c = cosf(ang);
        m11=c, m12=s, m13=0;
        m21=-s, m22=c, m23=0;
        m31=0, m32=0, m33=1;
        return *this;
    }

    Mat3& Mat3::setLookAt(const Vec3& from, const Vec3& to)
    {
        Vec3 forward = (to - from).normalized();
        float fdot   = Vec3::forward() | forward;
        if ( fabsf(fdot - (-1.0f)) < 0.0001f )
        {
            // Point in opposite directions
            *this = Mat3::angleAxis(Vec3::up(), PI);
        }
        else if ( fabsf(fdot - (1.0f)) < 0.0001f )
        {
            // Point in same direction
            *this = identity();
        }
        else
        {
            if ( fdot < -1.f ) fdot = -1.f;
            if ( fdot > 1.f )  fdot = 1.f;
            float ang = (float)acosf(fdot);
            Vec3 ax   = !(Vec3::forward() ^ forward);
            *this = Mat3::angleAxis(ax, ang);
        }
        return *this;
    }

    Mat3& Mat3::setFromAxis(const Vec3& x, const Vec3& y, const Vec3& z)
    {
        m11=x.x, m12=x.y, m13=x.z;
        m21=y.x, m22=y.y, m23=y.z;
        m31=z.x, m32=z.y, m33=z.z;
        return *this;
    }

    Mat3::operator Quat () const
    {
        float a = m11+m22+m33;
        float b = m11-m22-m33;
        float c = m22-m11-m33;
        float d = m33-m11-m22;

        int big = 0;
        float t = a;

        if ( b > t )
        {
            t = b;
            big=1;
        }
        if ( c > t )
        {
            t = c;
            big=2;
        }
        if ( d > t )
        {
            t = d;
            big=3;
        }

        float bv = sqrtf(t + 1) * .5f;
        float m = .25f / bv;

        switch ( big )
        {
        case 0: return { (m23-m32)*m, (m31-m13)*m, (m12-m21)*m, bv };
        case 1: return { bv, (m12+m21)*m, (m31+m13)*m, (m23-m32)*m };
        case 2: return { (m12+m21)*m, bv, (m23+m32)*m, (m31-m13)*m };
        case 3: return { (m31+m13)*m, (m23+m32)*m, bv, (m12-m21)*m };
        }

        return Quat::identity();
    }

    Mat3::operator Mat4() const
    {
        Mat4 m;
        m.m11=m11, m.m12=m12, m.m13=m13, m.m14=0;
        m.m21=m21, m.m22=m22, m.m23=m23, m.m24=0;
        m.m31=m31, m.m32=m32, m.m33=m33, m.m34=0;
        m.m41=0, m.m42=0, m.m43=0, m.m44=1;
        return m;
    }
}
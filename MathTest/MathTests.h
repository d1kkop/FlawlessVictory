#pragma once
#include "UnitTest.h"
#include "../Core.h"
using namespace fv;

UTESTBEGIN(QuatTransformTest)
{
    Vec3 v = { 1, 0, 0 };
    Quat q = Quat::rotateY( 90 * D2R );
    v = q * v;
    if ( fabsf(v.z + 1.f) > 0.001f ) return false;
    v = { 0, 1, 0 };
    q = Quat::rotateX( 90 * D2R );
    v = q * v;
    if ( fabsf( v.z -1.f ) > 0.01f ) return false;
    v = { 1, 0, 0 };
    q = Quat::rotateZ( 90 * D2R );
    v = q * v;
    if ( fabsf ( v.y - 1.f ) > 0.01f ) return false;
    return true;
}
UNITTESTEND(QuatTransformTest)

UTESTBEGIN(Mat3TransformTest)
{
    Vec3 v = { 1, 0, 0 };
    Mat3 q = Mat3::rotateY(90 * D2R);
    v = q * v;
    if ( fabsf(v.z + 1.f) > 0.001f ) return false;
    v = { 0, 1, 0 };
    q = Mat3::rotateX(90 * D2R);
    v = q * v;
    if ( fabsf(v.z -1.f) > 0.01f ) return false;
    v = { 1, 0, 0 };
    q = Mat3::rotateZ(90 * D2R);
    v = q * v;
    if ( fabsf (v.y - 1.f) > 0.01f ) return false;
    return true;
}
UNITTESTEND(Mat3TransformTest)

UTESTBEGIN(Mat4TransformTest)
{
    Vec3 v = { 1, 0, 0 };
    Mat4 q = Mat4::rotateY(90 * D2R);
    v = q * v;
    if ( fabsf(v.z + 1.f) > 0.001f ) return false;
    v = { 0, 1, 0 };
    q = Mat4::rotateX(90 * D2R);
    v = q * v;
    if ( fabsf(v.z -1.f) > 0.01f ) return false;
    v = { 1, 0, 0 };
    q = Mat4::rotateZ(90 * D2R);
    v = q * v;
    if ( fabsf (v.y - 1.f) > 0.01f ) return false;
    return true;
}
UNITTESTEND(Mat4TransformTest)

UTESTBEGIN(Mat3ToQuatTest)
{
    Mat3 m;
    Vec3 axis = { 1, 2, 4 };
    axis.normalize();
    m.setRotate( axis, 32 * D2R );
    Quat q  = (Quat)m;
    Mat3 m2 = (Mat3)q;
    for ( u32 i=0; i<9; ++i ) 
        if ( fabsf(m.cells[i]-m2.cells[i])>0.001f ) 
            return false;
    return true;
}
UNITTESTEND(Mat3ToQuatTest)

UTESTBEGIN(Mat4ToQuatTest)
{
    Mat4 m;
    Vec3 axis = { -1, 2.3f, -2.f };
    axis.normalize();
    m.setRotate(axis, 93.2f * D2R);
    Quat q  = (Quat)m;
    Mat4 m2 = (Mat4)q;
    for ( u32 i=0; i<16; ++i )
        if ( fabsf(m.cells[i]-m2.cells[i])>0.001f )
            return false;
    return true;
}
UNITTESTEND(Mat4ToQuatTest)

UTESTBEGIN(InverseMat3)
{
    Mat3 m;
    Vec3 axis = { -1, 2.3f, -2.f };
    axis.normalize();
    m.setRotate(axis, 93.2f * D2R);
    Mat3 mi = m.inverse() * m;
    Mat3 id = Mat3::identity();
    for ( u32 i=0; i<9; ++i )
        if ( fabsf(mi.cells[i]-id.cells[i]) > 0.001f )
            return false;
    return true;
}
UNITTESTEND(InverseMat3)

UTESTBEGIN(InverseMat4)
{
    Mat4 m;
    Vec3 axis = { -1, 2.3f, -2.f };
    axis.normalize();
    m.setRotate(axis, 93.2f * D2R);
    Mat4 mi = m.inverse() * m;
    Mat4 id = Mat4::identity();
    for ( u32 i=0; i<16; ++i )
        if ( fabsf(mi.cells[i]-id.cells[i]) > 0.001f )
            return false;
    return true;
}
UNITTESTEND(InverseMat4)

UTESTBEGIN(InverseQuat)
{
    Vec3 axis = { -1, 2.3f, -2.f };
    Vec3 axis2 = { -1, -2, -3.f };
    axis.normalize();
    axis2.normalize();
    Quat q  = Quat::angleAxis(axis, -2382);
    Quat q2 = Quat::angleAxis(axis2, -19);
    Quat qd = q.inverse() * q2;
    float ang = qd.angle();
    axis = qd.axis().normalized();
    Quat qd2 = q.conjugate() * q2;
    if ( fabsf(qd2.angle()-ang) > 0.001f ) 
        return false;
    axis2 = qd2.axis().normalized();
    if ( fabsf((axis|axis2)-1.f) > 0.001f ) 
        return false;
    return true;
}
UNITTESTEND(InverseQuat)

UTESTBEGIN(QuatLookAt)
{
    {
        Quat ql = Quat::lookAt(Vec3::zero(), Vec3::forward());
        Mat3 ml = (Mat3)ql;
        Vec3 fw = ml.axisZ();
        Vec3 ri = ml.axisX();
        Vec3 up = ml.axisY();
        if ( !fw.approx(Vec3::forward()) )
            return false;
        if ( !up.approx(Vec3::up()) )
            return false;
        if ( !ri.approx(Vec3::right()) )
            return false;
    }
    {
        Quat ql = Quat::lookAt(Vec3::zero(), Vec3::right());
        Mat3 ml = (Mat3)ql;
        Vec3 fw = ml.axisZ();
        Vec3 ri = ml.axisX();
        Vec3 up = ml.axisY();
        if ( !fw.approx(Vec3::right()) )
            return false;
        if ( !up.approx(Vec3::up()) )
            return false;
        if ( !ri.approx(-Vec3::forward()) )
            return false;
    }
    {
        Quat ql = Quat::lookAt(Vec3::zero(), Vec3::up());
        Mat3 ml = (Mat3)ql;
        Vec3 fw = ml.axisZ();
        Vec3 ri = ml.axisX();
        Vec3 up = ml.axisY();
        if ( !fw.approx(Vec3::up()) )
            return false;
        if ( !up.approx(-Vec3::forward()) )
            return false;
        if ( !ri.approx(Vec3::right()) )
            return false;
    }
    {
        Quat ql = Quat::lookAt(Vec3::zero(), -Vec3::forward());
        Mat3 ml = (Mat3)ql;
        Vec3 fw = ml.axisZ();
        Vec3 ri = ml.axisX();
        Vec3 up = ml.axisY();
        if ( !fw.approx(-Vec3::forward()) )
            return false;
        if ( !up.approx(Vec3::up()) )
            return false;
        if ( !ri.approx(-Vec3::right()) )
            return false;
    }
    return true;
}
UNITTESTEND(QuatLookAt)


UTESTBEGIN(Mat3LookAt)
{
    {
        Mat3 ml = Mat3::lookAt(Vec3::zero(), Vec3::forward());
        Vec3 fw = ml.axisZ();
        Vec3 ri = ml.axisX();
        Vec3 up = ml.axisY();
        if ( !fw.approx(Vec3::forward()) )
            return false;
        if ( !up.approx(Vec3::up()) )
            return false;
        if ( !ri.approx(Vec3::right()) )
            return false;
    }
    {
        Mat3 ml = Mat3::lookAt(Vec3::zero(), Vec3::right());
        Vec3 fw = ml.axisZ();
        Vec3 ri = ml.axisX();
        Vec3 up = ml.axisY();
        if ( !fw.approx(Vec3::right()) )
            return false;
        if ( !up.approx(Vec3::up()) )
            return false;
        if ( !ri.approx(-Vec3::forward()) )
            return false;
    }
    {
        Mat3 ml = Mat3::lookAt(Vec3::zero(), Vec3::up());
        Vec3 fw = ml.axisZ();
        Vec3 ri = ml.axisX();
        Vec3 up = ml.axisY();
        if ( !fw.approx(Vec3::up()) )
            return false;
        if ( !up.approx(-Vec3::forward()) )
            return false;
        if ( !ri.approx(Vec3::right()) )
            return false;
    }
    {
        Mat3 ml = Mat3::lookAt(Vec3::zero(), -Vec3::forward());
        Vec3 fw = ml.axisZ();
        Vec3 ri = ml.axisX();
        Vec3 up = ml.axisY();
        if ( !fw.approx(-Vec3::forward()) )
            return false;
        if ( !up.approx(Vec3::up()) )
            return false;
        if ( !ri.approx(-Vec3::right()) )
            return false;
    }
    return true;
}
UNITTESTEND(Mat3LookAt)


UTESTBEGIN(ComposeMat4)
{
    Vec3 p = { 5, -2.f, 1.f };
    Quat q = Quat::lookAt({ 2, 3, 4 }, { -2, 5, -1 });
    Vec3 s = { .2f, .3f, 2.4f };
    Mat4 m = Mat4::compose( p, q, s );
    Vec3 p2;
    Vec3 s2;
    Quat q2;
    m.decompose( p2, q2, s2 );
    if ( !p.approx(p2) )
        return false;
    if ( !q.approx(q2) )
        return false;
    if ( !s.approx(s2) )
        return false;
    return true;
}
UNITTESTEND(ComposeMat4)


UTESTBEGIN(Frustum)
{
    Mat4 m = Mat4::frustumRH( -1, 1, -1, 1, 1, 100 );
    Mat4 toLH = Mat4::identity();
    Mat4 m2 = toLH * m;
    Vec4 p = { -1, 0, 100, 1 };
    Vec4 pp = (m * p);
    float w = pp.w;
    pp /= pp.w;
    Vec4 po = m.inverse() * (pp*w);
    if ( !po.approx(p) )
        return false;
    return true;
}
UNITTESTEND(Frustum)
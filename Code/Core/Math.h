#pragma once
#include "Common.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Quat.h"
#include "Mat3.h"
#include "Mat4.h"
#include <cmath>

namespace fv
{
    constexpr float PI  = 3.1415926535897932f;
    constexpr float D2R = PI/180.f;
    constexpr float R2D = 180.f/PI;

    inline bool checkFloat(float f)                         { return std::isnormal(f)||f==0.f; }
    inline bool checkQuat(const Quat& q)                    { return checkFloat(q.x)||checkFloat(q.y)||checkFloat(q.z)||checkFloat(q.w); }
    inline bool checkVector(const Vec2& v)                  { return checkFloat(v.x)||checkFloat(v.y); }
    inline bool checkVector(const Vec3& v)                  { return checkFloat(v.x)||checkFloat(v.y)||checkFloat(v.z); }
    inline bool checkVector(const Vec4& v)                  { return checkFloat(v.x)||checkFloat(v.y)||checkFloat(v.z)||checkFloat(v.w); }
    inline bool checkMatrix(const Mat3& m)                  { return checkVector(m.axisX())||checkVector(m.axisY())||checkVector(m.axisZ()); }
    inline bool checkMatrix(const Mat4& m)                  { return checkVector(m.axisX())||checkVector(m.axisY())||checkVector(m.axisZ())||checkVector(m.translation()); }
}
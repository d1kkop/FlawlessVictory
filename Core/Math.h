#pragma once
#include "Common.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Quat.h"
#include "Mat3.h"
#include "Mat4.h"

namespace fv
{
    constexpr float PI  = 3.1415926535897932f;
    constexpr float D2R = PI/180.f;
    constexpr float R2D = 180.f/PI;

    FV_DLL u32 Hash32(const char* key, u32 len);
}
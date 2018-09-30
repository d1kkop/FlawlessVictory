#pragma once
#include "Common.h"

namespace fv
{
    // Returns local time as string.
    FV_DLL String localTime();

    // Format to string, accepts identical params as printf.
    FV_DLL String format(const char* msg, ...);

    // Time since epoch in seconds.
    FV_DLL double epochTime();

    // Time since start of application in seconds.
    FV_DLL float time();

    template <class T>
    void deleteAndNull(T*& ptr) { delete ptr; ptr = nullptr; }
}
#pragma once
#include "Common.h"

namespace fv
{
    // Returns local time as string.
    FV_DLL String LocalTime();

    // Format to string, accepts identical params as printf.
    FV_DLL String Format(const char* msg, ...);

    // Time since epoch in seconds.
    FV_DLL double EpochTime();

    // Time since start of application in seconds.
    FV_DLL float RunTime();

    // Returns 32 bits murmur hash of key.
    FV_DLL u32 Hash32(const char* key, u32 len);

    // Returns random integer.
    FV_DLL u32 Random();

    // Sleeps the thread for seconds.
    FV_DLL void Suspend(double seconds);

    FV_DLL void StringCopy(char* dst, u32 dstSize, const char* src);

    template <class T, class ...Args> 
    T* CreateOnce(T*& pType, Args... args) { if (!pType) pType=new T(args...); return pType; }
}
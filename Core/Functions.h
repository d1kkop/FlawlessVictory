#pragma once
#include "PCH.h"
#include "Common.h"

namespace fv
{
    template <class T> T Min(T a, T b) { return a<b?a:b; }
    template <class T> T Max(T a, T b) { return a>b?a:b; }
    template <class T> T Clamp(T a, T low, T high) { return Min(high, Max(a, low)); }

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

    // Allocates a new char array and copies from srouce. Len is optional.
    FV_DLL const char* NewString( const char* src, u32 len=-1 );

    template <class T, class ...Args> 
    T* CreateOnce(T*& pType, Args... args) { if (!pType) pType=new T(args...); return pType; }
}
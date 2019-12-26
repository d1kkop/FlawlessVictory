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

    // Inclusive min, exclusive max.
    FV_DLL u32 Random(u32 min, u32 max);

    // Sleeps the thread for seconds.
    FV_DLL void Suspend(double seconds);

    // Wrapper around strcpy or strcpy_s.
    FV_DLL void StringCopy(char* dst, u32 dstSize, const char* src);

    // Allocates a new char array and copies from srouce. Len is optional.
    FV_DLL const char* NewString( const char* src, u32 len=-1 );

    // Read at once contents of file as binary.
    FV_DLL bool LoadBinaryFile(const char* path, Vector<char>& data);

    // Opens a file.
    FV_DLL bool OpenFile( FILE*& file, const char* path, const char* mode );

    // Closes a file.
    FV_DLL void CloseFile( FILE* file, bool flush = false );

    // Returns file's last modified time.
    FV_DLL u64 FileModifiedTime( const char* path );

    // Useful for destructors that depend on manager that may not exist anymore.
    FV_DLL bool IsEngineClosing();

    // Private function! Only to be called from engine.
    FV_DLL void SetEngineClosing(); 

    template <class T, class ...Args> 
    T* CreateOnce(T*& pType, Args... args) { if (!pType) pType=new T(args...); return pType; }
}
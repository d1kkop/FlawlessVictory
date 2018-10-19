#pragma once

#if FV_INCLUDE_WINHDR
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #undef min
    #undef max
#endif
#pragma once

#define FV_ST /* A function that cannot be called from updateMT! */
#define FV_TS /* A thread safe function. */
#define FV_TRACEJOBSYSTEM 0

#if _DEBUG
#define FV_DEBUG 1
#endif

#ifdef FV_EXPORTING
    #define FV_DLL __declspec(dllexport)
#elif FV_IMPORTING
    #define FV_DLL __declspec(dllimport)
#endif

// Configuration
#define FV_EDITOR 1
#define FV_SDL 1

// API Constants
#define FV_NO_IMPLEMENTATION_ERR (-9999)
#define FV_INVALID_IDX (-1)

// Platform
#if _MSC_VER
#define FV_SECURE_CRT 1
#define FV_INCLUDE_WINHDR 1
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define FV_FUNCTION __FUNCTION__
#define FV_LINE		__LINE__
#define FV_FL		MM_FUNCTION, MM_LINE
#endif


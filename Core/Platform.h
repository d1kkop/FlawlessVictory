#pragma once

#ifdef FV_EXPORTING
    #define FV_DLL __declspec(dllexport)
#elif FV_IMPORTING
    #define FV_DLL __declspec(dllimport)
#endif

/* General */
#define FV_SECURE_CRT 1
#define FV_INCLUDE_WINHDR 0
#define FV_FUNCTION __FUNCTION__
#define FV_LINE		__LINE__
#define FV_FL		MM_FUNCTION, MM_LINE
#define FV_NO_IMPLEMENTATION_ERR (-9999)
#define FV_INVALID_IDX (-1)
#define FV_EDITOR 1
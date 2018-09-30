#pragma once

#define FV_ST /* A function that cannot be called from updateMT! */

#if _DEBUG
#define FV_DEBUG 1
#endif

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

#define FV_SDL 1

#pragma once

#define FV_MO /* A mono function that can only be called from main thread. */
#define FV_TS /* A thread safe function. Can be called from any thread. */
#define FV_TC /* Thread safe in a sense that the data returned from this function (is constant and) is set from the constructor (or a function called from constructor). */
#define FV_BG /* A function that must be called before begin fase has marked over. */

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
#define FV_MAX_EXTENSION 16
#define FV_USEJOBSYSTEM 1

// Platform enables/disables
#if _WIN32
    #define FV_SDL 1
    #define FV_STB 0
    #define FV_FREEIMAGE 1
    #define FV_VULKAN  1
    #define FV_GLSL2SPIRVLIB 0
    #define FV_GLSL2SPIRVBIN 1
    #define FV_VLD 0
    #define FV_ASSIMP 1
    #define FV_NLOHMANJSON 1
    #define FV_INCLUDE_WINHDR 1
#endif

// Compiler mappings
#if _MSC_VER
    #define FV_SECURE_CRT 1
    #if !FV_SECURE_CRT
        #define _CRT_SECURE_NO_WARNINGS
    #endif
    #define FV_FUNCTION __FUNCTION__
    #define FV_LINE		__LINE__
    #define FV_FL		FV_FUNCTION, FV_LINE
#endif
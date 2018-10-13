#pragma once

#define FV_MO /* A mono function that cannot be called from updateMT! */
#define FV_TS /* A thread safe function. Can be called from anywhere. */
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


#if FV_INCLUDE_WINHDR
    #define WIN32_LEAN_AND_MEAN
    #undef min
    #undef max
    #include <windows.h>
#endif

#if FV_SDL
#include "../3rdParty/SDL/include/SDL.h"
#undef main
#if _MSC_VER
#pragma comment(lib, "../3rdParty/SDL/lib/x64/SDL2.lib")
#endif
#endif

#if FV_STB
#include "../3rdParty/stb-master/stb_image.h"
#endif

#if FV_FREEIMAGE
#include "../3rdParty/FreeImage/Dist/x64/FreeImage.h"
#if _MSC_VER
#pragma comment(lib, "../3rdParty/FreeImage/Dist/x64/FreeImage.lib")
#endif
#endif

#if FV_VLD
#include "../3rdParty/Visual Leak Detector/include/vld.h"
#pragma comment(lib, "../3rdParty/Visual Leak Detector/lib/Win64/vld.lib")
#endif

#if FV_ASSIMP
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma comment(lib, "../3rdParty/Assimp/lib/x64/assimp-vc140-mt.lib")
#endif
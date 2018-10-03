#pragma once
#include "Common.h"

namespace fv
{
    FV_DLL bool OSInitialize();
    FV_DLL void OSShutdown();
    FV_DLL bool OSLoadLibrary(const char* path);
    FV_DLL void OSSetThreadName(const char* name);
    FV_DLL void* OSCreateWindow(const char* name, u32 posX, u32 posY, u32 width, u32 height, bool fullscreen);
    FV_DLL void OSDestroyWindow(void* pWindow);
    FV_DLL void OSSwapWindow(void* pWindow);
}
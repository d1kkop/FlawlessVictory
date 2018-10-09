#pragma once
#include "Common.h"

namespace fv
{
    struct OSHandle
    {
        union
        {
            char data[64];
            struct { void* process, * thread; };
            void* library;
            void* function;
            void* window;
        };
    };

    FV_DLL bool OSInitialize();
    FV_DLL void OSShutdown();
    FV_DLL OSHandle OSLoadLibrary(const char* path);
    FV_DLL void OSSetThreadName(const char* name);
    FV_DLL void* OSCreateWindow(const char* name, u32 posX, u32 posY, u32 width, u32 height, bool fullscreen, bool supportVulkan, bool supportOpenGL);
    FV_DLL void OSDestroyWindow(void* pWindow);
    FV_DLL OSHandle OSStartProgram(const char* path, const char* arguments, bool waitToFinish);
    FV_DLL void OSWaitOnProgram(OSHandle handle);
    FV_DLL OSHandle OSFindFunction(OSHandle libHandle, const char* name);
}
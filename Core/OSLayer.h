#pragma once
#include "Platform.h"

namespace fv
{
    FV_DLL bool OSInitialize();
    FV_DLL void OSShutdown();
    FV_DLL bool OSLoadLibrary(const char* path);
    FV_DLL void OSSetThreadName(const char* name);
}
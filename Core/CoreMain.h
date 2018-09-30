#pragma once
#include "Platform.h"
#include "LogManager.h"

#define FV_CHECK_ST() \
{\
    if ( !componentManager()->executingSingleThreaded()) \
    { \
        LOGC("Executing single threaded function from parallel. Function call ignored."); \
        assert(false && "Executing single threaded function from parallel. Function call ignored."); \
    }\
}

namespace fv
{
    FV_DLL void core_start();
    FV_DLL void core_close();

    FV_DLL class TypeManager* typeManager();
    FV_DLL class ComponentManager* componentManager();
    FV_DLL class LogManager* logManager();
}
#pragma once
#include "Platform.h"

namespace fv
{
    FV_DLL void core_start();
    FV_DLL void core_close();

    FV_DLL class TypeManager* typeManager();
    FV_DLL class ComponentManager* componentManager();
    FV_DLL class LogManager* logManager();
}
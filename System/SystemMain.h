#pragma once
#include "../Core/Platform.h"

namespace fv
{
    FV_DLL void system_start();
    FV_DLL void system_close();

    class SystemManager* systemManager();
}
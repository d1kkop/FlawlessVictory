#pragma once
#include "Common.h"

namespace fv
{
    struct FV_DLL Directories
    {
        static Path assets();
        static Path build();
        static Path intermediate();
        static Path log();
    };
}
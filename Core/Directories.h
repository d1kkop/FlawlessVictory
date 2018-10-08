#pragma once
#include "PCH.h"

namespace fv
{
    struct FV_DLL Directories
    {
        static Path bin();
        static Path assets();
        static Path build();
        static Path intermediate();
        static Path log();
        static Path intermediateShaders();
    };
}
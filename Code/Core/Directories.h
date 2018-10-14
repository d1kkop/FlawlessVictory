#pragma once
#include "PCH.h"

namespace fv
{
    struct FV_DLL Directories
    {
        // Root
        static const Path& bin();
        static const Path& assets();
        static const Path& build();
        static const Path& intermediate();
        static const Path& log();
        static const Path& standard();
        // Intermediate
        static const Path& intermediateShaders();
        static const Path& intermediateMeshes();
        static const Path& intermediateTextures();
    };
}
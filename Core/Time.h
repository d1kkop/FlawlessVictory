#pragma once
#include "Functions.h"

namespace fv
{
    /*  The time struct is updated before each new frame. */
    struct FV_DLL Time
    {
        // Time since epoch in seconds.
        static double epoch();

        // Delta time since last frame in seconds.
        static float dt();

        // Elapsed time since first update call in seconds.
        static float elapsed();

        // Physics delta time. This will not vary.
        static float physicsDt();

        // Network delta time. This will not vary.
        static float networkDt();

    private:
        static void start();
        static void update();

        friend class SystemManager;
    };
}
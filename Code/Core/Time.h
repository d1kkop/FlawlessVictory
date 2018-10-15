#pragma once
#include "Functions.h"

namespace fv
{
    /*  The time struct is updated before each new frame. */
    struct FV_DLL Time
    {
        // Current frame number. First frame is 0.
        static u64 frameNumber();

        // Number of frames per second.
        static u32 frameRate();

        // Frame time in milliseconds.
        static float frameTime();

        // Delta time since last frame in seconds. Updated once a frame.
        static float dt();

        // Elapsed time since first update call in seconds. Updated once a frame.
        static float elapsed();

        // Physics delta time. This will not vary.
        static float physicsDt();

        // Network delta time. This will not vary.
        static float networkDt();
    };


    // Called by engine.
    FV_DLL void TimeStart();
    FV_DLL void TimeUpdate();
}
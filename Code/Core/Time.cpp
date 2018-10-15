#include "PCH.h"
#include "Time.h"

namespace fv
{
    u64    g_FrameNumber = 0;
    u32    g_FrameRate = 0;
    float  g_FrameTime = 0;
    float  g_LastTime = 0;
    double g_Epoch = 0;
    double g_EpochBegin = 0;
    float  g_Dt = 0;
    float  g_Elapsed = 0;
    float  g_PhysicsDt = 1.f/30.f;
    float  g_NetworkDt = 1.f/20.f;

    // Helpers
    u32 g_Ticks = 0;
    double g_LastFpsMeasurement = 0;

    u64 frameNumber()           { return g_FrameNumber; }
    u32 frameRate()             { return g_FrameRate; }
    float Time::frameTime()     { return g_FrameTime; }
    float Time::dt()            { return g_Dt; }
    float Time::elapsed()       { return g_Elapsed; }
    float Time::physicsDt()     { return g_PhysicsDt; }
    float Time::networkDt()     { return g_NetworkDt; }

    void TimeStart()
    {
        g_Epoch = EpochTime();
        g_EpochBegin = g_Epoch;
        g_LastFpsMeasurement = g_Epoch;
    }

    void TimeUpdate()
    {
        double prevEpoch = g_Epoch;
        g_Epoch = EpochTime();
        g_Dt = (float)(g_Epoch - prevEpoch);
        g_Elapsed = (float)(g_Epoch - g_EpochBegin);
        ++g_Ticks;
        if ( g_Epoch - g_LastFpsMeasurement >= 1 )
        {
            g_FrameRate = g_Ticks;
            g_FrameTime = 1000.f / g_Ticks;
            g_LastFpsMeasurement = g_Epoch;
            g_Ticks = 0;
        }
    }

}

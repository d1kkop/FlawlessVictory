#include "PCH.h"
#include "Time.h"

namespace fv
{
    float g_lastTime = 0;
    double g_epoch = 0;
    double g_epochBegin = 0;
    float g_dt = 0;
    float g_elapsed = 0;
    float g_physicsDt = 1.f/30.f;
    float g_networkDt = 1.f/20.f;

    double Time::epoch()        { return g_epoch; }
    float Time::dt()            { return g_dt; }
    float Time::elapsed()       { return g_elapsed; }
    float Time::physicsDt()     { return g_physicsDt; }
    float Time::networkDt()     { return g_networkDt; }

    void Time::start()
    {
        g_epoch = epoch();
        g_epochBegin = g_epoch;
    }

    void Time::update()
    {
        double prevEpoch = g_epoch;
        g_dt = (float)(g_epoch - prevEpoch);
        g_elapsed = (float)(g_epoch - g_epochBegin);
    }

}

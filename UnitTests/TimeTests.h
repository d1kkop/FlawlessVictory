#pragma once
#include "UnitTest.h"
#include "../Core.h"
#include <iostream>
using namespace fv;

UTESTBEGIN(TimeTest)
{
    double k = EpochTime();
    double k2 = EpochTime();
    float s  = RunTime();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    float s2 = RunTime();
    std::cout << "Time was " << (s2-s) << std::endl;
    return true;
}
UNITTESTEND(TimeTest)

UTESTBEGIN(BenchmarkQuatLock)
{
    u32 numIters = 1;
    Quat q = Quat::identity();
    Vec3 ax = {.1f, 1.2f, 3.f};
    Quat r = Quat::angleAxis(ax.normalized(), 231 * D2R);
    double s, s2;
    {
        s = RunTime();
        for ( u32 i =0; i < numIters; ++i )
        {
            q *= r;
        }
        s2 = RunTime();
    }
    printf("Q = %3.f %.3f %.3f %.3f\n", q.x, q.y, q.z, q.w);
    printf("Time taken (NO LOCK) is %lf ms\n", (s2-s)*1000);
    Mutex m;
    s = RunTime();
    { 
        scoped_lock lk(m);
        for ( u32 i =0; i < numIters; ++i )
        {
            q *= r;
        }
    }
    s2 = RunTime();
    printf("Time taken (WITH LOCK) is %lf ms\n", (s2-s)*1000);
    printf("Q = %3.f %.3f %.3f %.3f\n", q.x, q.y, q.z, q.w);
    return true;
}
UNITTESTEND(BenchmarkQuatLock)
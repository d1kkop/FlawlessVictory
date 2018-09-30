#pragma once
#include "UnitTest.h"
#include "../Core.h"
#include <iostream>
using namespace fv;

UTESTBEGIN(TimeTest)
{
    double k = epochTime();
    double k2 = epochTime();
    float s  = time();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    float s2 = time();
    std::cout << "Time was " << (s2-s) << std::endl;
    return true;
}
UNITTESTEND(TimeTest)
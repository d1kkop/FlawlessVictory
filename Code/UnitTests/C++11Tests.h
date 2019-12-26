#pragma once
#include "UnitTest.h"
#include <cassert>
using namespace fv;

class CppClass
{
public:
    int* pActive;//= {};
    String k;

    CppClass() = default;

    CppClass(String s):
        k(s)
    {
    }
};

class CppClass2
{
public:
    int* pActive;
};

UTESTBEGIN(CheckDefaultInitialization)
{
    CppClass cpp;
    assert( cpp.pActive == nullptr );
    CppClass cpp2("tralala");
    assert( cpp.pActive == nullptr );
    return true;
}
UNITTESTEND( CheckDefaultInitialization )

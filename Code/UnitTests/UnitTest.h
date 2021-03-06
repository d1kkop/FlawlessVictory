#pragma once

#include "../Core.h"
using namespace fv;


namespace UnitTest
{
    class UnitTestBase
    {
    public:
        UnitTestBase(const char* name): m_Name(name) { }
        virtual bool run() = 0;
        String name() { return m_Name; }
        String m_Name;
    };

    Vector<UnitTestBase*> g_UnitTests;
    UnitTestBase* addUTest(UnitTestBase* ut) { g_UnitTests.push_back(ut); return ut; }
}


#define UTESTBEGIN(name) \
namespace UnitTest\
{\
	class UTest##name : public UnitTestBase \
	{\
public:\
		UTest##name(): UnitTestBase(#name) { } \
		bool run() override

#define UNITTESTEND(name)\
	};\
}\
static UnitTest::UTest##name * st_##name = static_cast<UnitTest::UTest##name*>( UnitTest::addUTest( new UnitTest::UTest##name() ) );

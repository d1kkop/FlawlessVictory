#include "UnitTest.h"
#include "MemoryTests.h"
#include "MathTests.h"
#include "TimeTests.h"
#include "Bechmarks.h"
#include "GraphicTests.h"
#include "../Core/Reflection.h"
#include "../Core/Component.h"
#include <iostream>
using namespace std;
using namespace UnitTest;
using namespace fv;

extern "C"
{
    FV_DLL void entry(i32 argc, char** arv)
    {
        for ( auto* ut : g_UnitTests )
        {
            //     if ( ut->name() != "BenchLoopOverComponents3") // For testing a single unit
            //         continue;

            if ( ut->run() )
            {
                cout << ut->name() << " Fine" << endl;
            }
            else
            {
                cout << ut->name() << " FAILED" << endl;
            }
            delete ut;
        }
        system("pause");
    }
}
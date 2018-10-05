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

class UnitTestComponent: public GameComponent
{
    FV_TYPE(UnitTestComponent, 0, false)

    void begin() override
    {
        for ( auto* ut : g_UnitTests )
        {
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
};
FV_TYPE_IMPL(UnitTestComponent)
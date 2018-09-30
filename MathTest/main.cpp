#include "UnitTest.h"
#include "MemoryTests.h"
#include "MathTests.h"
#include "TimeTests.h"
#include <iostream>
using namespace std;
using namespace UnitTest;

int main(int argc, char** arv)
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

    return 0;
}
#include "UnitTest.h"
#include "MathTests.h"
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
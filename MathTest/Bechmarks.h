#pragma once
#include "UnitTest.h"
#include "../Core.h"
#include "../Scene/ComponentManager.h"
#include <cassert>
using namespace fv;

class BenchComponent: public Component
{
    FV_TYPE(BenchComponent, 0, false)

    void update(float dt) override
    {
        product = a * b;
    }

    float product;
    float a = 3;
    float b = 3;
};
FV_TYPE_IMPL(BenchComponent)


UTESTBEGIN(BenchLoopOverComponents)
{
    printf("10 Times loop over scattered and contiguous memory\n");
    for ( u32 t = 0; t < 10; t ++ )
    {
        u32 numItrs=1000000;
        Vector<BenchComponent*> benchComps;
        volatile char* garbage = new volatile char[1024];
        for ( u32 i=0; i<numItrs; ++i )
        {
            BenchComponent* comp = new BenchComponent;
            delete [] garbage; 
            garbage = new char[1024];
            benchComps.emplace_back( comp );
        }
        float s, s2;
        s = RunTime();
        for ( u32 i=0; i<numItrs; ++i )
        {
            Component*c = benchComps[i];
            c->update( 0 );
        }
        s2 = RunTime();
        printf("Loop over scattered memory %.3f us\n", (s2-s)*1000.f*1000.f);
        for ( u32 i=0; i<numItrs; ++i )
        {
            componentManager()->newComponent<BenchComponent>();
        }
        s = RunTime();
        for ( Component& c : Itr<BenchComponent>() )
        {
            c.update( 0 );
        }
        s2 = RunTime();
        printf("Loop over contiguous array %.3f us\n", (s2-s)*1000.f*1000.f);
        for ( auto* c : benchComps ) delete c;
        componentManager()->freeAllOfType<BenchComponent>();
    }
    return true;
}
UNITTESTEND(BenchLoopOverComponents)
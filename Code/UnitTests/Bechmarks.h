#pragma once
#include "UnitTest.h"
#include "../Core.h"
#include "../Core/ComponentManager.h"
#include "../Core/ComponentManager2.h"
#include "../Scene/GameComponent.h"
#include <cassert>
using namespace fv;

class BenchComponent: public Component
{
    FV_TYPE(BenchComponent)

    void multiply()
    {
        for ( u32 i =0; i < 64; ++i )
        {
            product *= a;
            product /= b;
            product += a;
            product *= (a-b);
        }
        
    }

    float product;
    float a = 3.3f;
    float b = 23.12f;
};
FV_TYPE_IMPL(BenchComponent)


UTESTBEGIN(BenchLoopOverComponents)
{
    printf("10 Times loop over scattered and contiguous memory\n");
    for ( u32 t = 0; t < 10; t ++ )
    {
        u32 numItrs=1000000;
    #if FV_DEBUG
            numItrs=1000;
    #endif
        Vector<BenchComponent*> benchComps;
        volatile char* garbage = new volatile char[1024];
        for ( u32 i=0; i<numItrs; ++i )
        {
            BenchComponent* comp = new BenchComponent;
            delete [] garbage; 
            garbage = new volatile char[1024];
            benchComps.emplace_back( comp );
        }
        float s, s2;
        s = RunTime();
        for ( u32 i=0; i<numItrs; ++i )
        {
            BenchComponent*c = benchComps[i];
            c->multiply();
        }
        s2 = RunTime();
        printf("Loop over scattered memory %.3f us\n", (s2-s)*1000.f*1000.f);
        for ( u32 i=0; i<numItrs; ++i )
        {
            componentManager()->newComponent<BenchComponent>();
        }
        s = RunTime();
        for ( BenchComponent& c : Itr<BenchComponent>() )
        {
            c.multiply();
        }
        s2 = RunTime();
        printf("Loop over contiguous array %.3f us\n", (s2-s)*1000.f*1000.f);
        for ( auto* c : benchComps ) delete c;
        componentManager()->freeAllOfType<BenchComponent>();
    }
    return true;
}
UNITTESTEND(BenchLoopOverComponents)

UTESTBEGIN(BenchLoopOverComponents2)
{
    printf("10 Times loop over scattered and contiguous memory\n");
    for ( u32 t = 0; t < 10; t++ )
    {
        u32 numItrs=1000000;
    #if FV_DEBUG
        numItrs=1000;
    #endif
        double s, s2;
        for ( u32 i=0; i<numItrs; ++i )
        {
            componentManager()->newComponent<BenchComponent>();
        }
        s = RunTime();
        for ( BenchComponent& c : Itr<BenchComponent>() )
        {
            c.multiply();
        }
        s2 = RunTime();
        printf("Loop over contiguous array %.3f us\n", (s2-s)*1000.f*1000.f);
        componentManager()->freeAllOfType<BenchComponent>();
        // -----------------------------------------------------------------------------------
        for ( u32 i=0; i<numItrs; ++i )
        {
            componentManager2()->newComponent<BenchComponent>();
        }
        s = RunTime();
        for ( BenchComponent& c : Itr2<BenchComponent>() )
        {
            c.multiply();
        }
        s2 = RunTime();
        printf("Loop over shared ptr contiguous array %.3f us\n", (s2-s)*1000.f*1000.f);
        componentManager2()->freeAllOfType<BenchComponent>();
    }
    return true;
}
UNITTESTEND(BenchLoopOverComponents2)



UTESTBEGIN(BenchLoopOverComponents3)
{
    printf("10 Times loop over scattered and contiguous memory\n");
    for ( u32 t = 0; t < 10; t++ )
    {
        u32 numItrs=1000000;
    #if FV_DEBUG
        numItrs=1000;
    #endif
        double s, s2;
        for ( u32 i=0; i<numItrs; ++i )
        {
            componentManager()->newComponent<BenchComponent>();
        }
        s = RunTime();
        for ( BenchComponent& c : Itr<BenchComponent>() )
        {
            c.multiply();
        }
        s2 = RunTime();
        printf("Loop over contiguous array (ST) %.3f us\n", (s2-s)*1000.f*1000.f);
        componentManager()->freeAllOfType<BenchComponent>();
        // -----------------------------------------------------------------------------------
        for ( u32 i=0; i<numItrs; ++i )
        {
            componentManager()->newComponent<BenchComponent>();
        }
        s = RunTime();
        ParallelComponentFor<BenchComponent>( componentManager()->componentsOfType<BenchComponent>(), [](BenchComponent& bc)
        {
            bc.multiply();
        });
        s2 = RunTime();
        printf("Loop over contiguous array (MT) %.3f us\n", (s2-s)*1000.f*1000.f);
        componentManager()->freeAllOfType<BenchComponent>();
    }
    return true;
}
UNITTESTEND(BenchLoopOverComponents3)

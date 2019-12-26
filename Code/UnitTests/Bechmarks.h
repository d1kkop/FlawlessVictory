#pragma once
#include "UnitTest.h"
#include "../Core.h"
#include "../Core/ComponentManager.h"
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
    printf("10 Times loop over scattered and contiguous memory (rawPtr)\n");
    for ( u32 t = 0; t < 10; t ++ )
    {
        u32 numItrs=100000;
    #if FV_DEBUG
            numItrs=1000;
    #endif
        Vector<BenchComponent*> benchComps;
        Vector<volatile char*> garbages;
        for ( u32 i=0; i<numItrs; ++i )
        {
            BenchComponent* comp = new BenchComponent;
            volatile char* garbage = new volatile char[Random(1024,1283)];
            benchComps.emplace_back( comp );
            garbages.emplace_back( garbage );
        }
        float s, s2;
        s = RunTime();
        for ( u32 i=0; i<numItrs; ++i )
        {
            BenchComponent*c = benchComps[i];
            c->multiply();
        }
        s2 = RunTime();
        for ( auto g : garbages ) delete [] g;
        printf("Loop over 'scattered' memory %.3f us\n", (s2-s)*1000.f*1000.f);
        List<M<BenchComponent>> comps;
        for ( u32 i=0; i<numItrs; ++i )
        {
            comps.emplace_back( componentManager()->newComponent<BenchComponent>() );
        }
        s = RunTime();
        for ( BenchComponent& c : Itr<BenchComponent>() )
        {
            c.multiply();
        }
        s2 = RunTime();
        printf("Loop over contiguous array %.3f us\n", (s2-s)*1000.f*1000.f);
        for ( auto* c : benchComps ) delete c;
        comps.clear();
        componentManager()->freeAllMemory();
    }
    return true;
}
UNITTESTEND(BenchLoopOverComponents)

UTESTBEGIN(BenchLoopOverComponents2)
{
    printf("10 Times loop over scattered and contiguous memory (shrdPtr)\n");
    for ( u32 t = 0; t < 10; t++ )
    {
        u32 numItrs=100000;
    #if FV_DEBUG
        numItrs=1000;
    #endif
        double s, s2;
        List<M<BenchComponent>> comps;
        List<volatile char*> garbages;
        for ( u32 i=0; i<numItrs; ++i )
        {
            comps.emplace_back( componentManager()->newComponent<BenchComponent>() );
            volatile char* garbage = new char[Random(1024,1123)];
            garbages.emplace_back( garbage );
        }
        s = RunTime();
        for ( BenchComponent& c : Itr<BenchComponent>() )
        {
            c.multiply();
        }
        s2 = RunTime();
        comps.clear();
        for ( auto g : garbages ) delete [] g;
        printf("Loop over contiguous array %.3f us\n", (s2-s)*1000.f*1000.f);
        componentManager()->freeAllMemory();
        // -----------------------------------------------------------------------------------
        for ( u32 i=0; i<numItrs; ++i )
        {
            comps.emplace_back( componentManager()->newComponent<BenchComponent>() );
        }
        s = RunTime();
        for ( BenchComponent& c : Itr<BenchComponent>() )
        {
            c.multiply();
        }
        s2 = RunTime();
        printf("Loop over shared ptr contiguous array %.3f us\n", (s2-s)*1000.f*1000.f);
        comps.clear();
        componentManager()->freeAllMemory();
    }
    return true;
}
UNITTESTEND(BenchLoopOverComponents2)



UTESTBEGIN(BenchLoopOverComponents3)
{
    printf("10 Times loop over scattered and contiguous memory\n");
    for ( u32 t = 0; t < 4; t++ )
    {
        u32 numItrs=400000;
    #if FV_DEBUG
        numItrs=1000;
    #endif
        double s, s2;
        List<M<BenchComponent>> comps;
        for ( u32 i=0; i<numItrs; ++i )
        {
            comps.emplace_back( componentManager()->newComponent<BenchComponent>() );
        }
        s = RunTime();
        for ( BenchComponent& c : Itr<BenchComponent>() )
        {
            c.multiply();
        }
        s2 = RunTime();
        printf("Loop over contiguous array (ST) %.3f us\n", (s2-s)*1000.f*1000.f);
        comps.clear();
        componentManager()->freeAllMemory();
        // -----------------------------------------------------------------------------------
        for ( u32 i=0; i<numItrs; ++i )
        {
            comps.emplace_back( componentManager()->newComponent<BenchComponent>() );
        }
        double tt, t2;
        tt = RunTime();
        ParallelComponentFor<BenchComponent>( componentManager()->componentsOfType<BenchComponent>(), [](BenchComponent& bc, u32 tIdx)
        {
            bc.multiply();
        });
        t2 = RunTime();
        printf("Loop over contiguous array (MT) %.3f us | MT %.3fX faster than ST\n", (t2-tt)*1000.f*1000.f, 1.0/((t2-tt)/(s2-s)));
        comps.clear();
        componentManager()->freeAllMemory();
    }
    return true;
}
UNITTESTEND(BenchLoopOverComponents3)

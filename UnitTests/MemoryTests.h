#pragma once
#include "UnitTest.h"
#include "../Core.h"
#include "../Core/JobManager.h"
#include "../Core/ComponentManager.h"
#include "../Scene/GameComponent.h"
#include <cassert>
using namespace fv;

class TestComponent: public GameComponent
{
    FV_TYPE(TestComponent, 0, false)

    TestComponent()
    {
        // Ensure recycled objects have their default state
        assert( TestMap.size() == 0 );
        assert( s == "Hello TestComponent" );
    }

    ~TestComponent() override 
    {
        static int k = 0;
    //    printf("TestComp destr called %d\n", k++);
    }
    

public:
    Map<String, String> TestMap;
    String s = "Hello TestComponent";
};
FV_TYPE_IMPL(TestComponent)

class TestUpdComponent: public TestComponent
{
    FV_TYPE(TestUpdComponent, 0, false)
public:
    bool updatable() const override { return true; }

    ~TestUpdComponent() override { }
};
FV_TYPE_IMPL(TestUpdComponent)


UTESTBEGIN(ComponentManagerTest)
{
    for ( u32 j=0; j<10; j++) // See if components are reused on sequential iterations
    {
        u32 k=1000;
        Vector<TestComponent*> comps;
        comps.reserve(k);
        for ( u32 i=0; i<k; ++i )
        {
            comps.emplace_back(componentManager()->newComponent<TestComponent>());
            comps.back()->TestMap["Aap"] = "Beer";
            comps.back()->s = "Changed string 2.";
            comps.emplace_back(componentManager()->newComponent<TestUpdComponent>());
            comps.back()->TestMap["Beer"] = "Aap";
            comps.back()->s = "Changed string.";
        }
        u32 numComps = componentManager()->numComponents();
        assert( numComps ==k*2 );
        while ( comps.size() )
        {
            u32 r = rand() % comps.size();
            componentManager()->freeComponent(comps[r]);
            comps.erase(comps.begin() + r);
        }
     //   printf("After free -> Num components %d \n", componentManager()->numComponents() );
    }
    deleteComponentManager();
    return true;
}
UNITTESTEND(ComponentManagerTest)


UTESTBEGIN(IterTest)
{
    // Test 0 elements
    u32 g = 0;
    for ( TestUpdComponent& tc : Itr<TestUpdComponent>() )
    {
        g++;
    }
    assert(g == 0);

    componentManager()->newComponent<TestUpdComponent>();
    componentManager()->newComponent<TestUpdComponent>();
    componentManager()->newComponent<TestUpdComponent>();

    // Should be 3 now
    g = 0;
    for ( TestUpdComponent& tc : Itr<TestUpdComponent>() )
    {
        g++;
    }
    assert( g == 3 );

    // Test when bordering the buffer size of X (eg 128)
    g = 0;
    for ( u32 i=0; i<1000; i++ )
    {
        auto* c = componentManager()->newComponent<TestComponent>();
        c->s = "Hello Iterator";
        g++;
    }
    assert( g==1000 );

    // Test if mapping to derived component is correct (no invalid ptr offsets)
    for ( auto& tc : Itr<TestComponent>() )
    {
        assert( tc.s == "Hello Iterator" );
    }
    return true;

    componentManager()->freeAllOfType<TestComponent>();
    componentManager()->freeAllOfType<TestUpdComponent>();

    assert( componentManager()->numComponents<TestComponent>() == 0 );
    assert( componentManager()->numComponents<TestUpdComponent>() == 0 );
}
UNITTESTEND(IterTest)

Atomic<u32> g_JobRecursion = 0;
void addJob2AndWait();
void addJob1AndWait()
{
    g_JobRecursion++;
    if ( g_JobRecursion > 300 /* Watch out for stack overflow*/ )
        return;

    u32 v = g_JobRecursion;
 //   printf("Job %d executed \n", v);

    jobManager()->addJob( addJob2AndWait )->waitAndFree();
}
void addJob2AndWait()
{
    addJob1AndWait();
}

UTESTBEGIN(JobManagerRecursionTest)
{
    for ( u32 i=0; i<10; i++ )
    {
        g_JobRecursion = 0;
        jobManager()->addJob( addJob1AndWait )->waitAndFree();
    }
    return true;
}
UNITTESTEND(JobManagerRecursionTest)


void addJob2();
void addJob1()
{
    u32 v = g_JobRecursion++;
    if ( v > 300 /*Watch out for stack overflow*/ )
        return;

    // Do random, wait/cancel
    if ( Random() % 200 == 0 )
        jobManager()->addJob([v]() {
          //  printf("Job %d executed \n", v);        
            addJob2(); 
    })->waitAndFree();
    else
    {
        Job* jChild = jobManager()->addJob([v]() {
         //   printf("Job %d executed \n", v);
            addJob2();
        });

        Suspend(0.0001);
        if ( Random() % 2 == 0 )
           if ( jChild->cancelAndFree() ) { /* printf("Job %d cancelled\n", v);*/ }
        else
           if ( jChild->cancel() ) { /* printf("Job %d cancelled\n", v); */ }
    }
}
void addJob2()
{
    addJob1();
}

UTESTBEGIN(JobManagerNonRecursionTest)
{
    for ( u32 i=0; i<10; i++ )
    {
        g_JobRecursion = 0;
        jobManager()->addJob(addJob1)->waitAndFree();
    }
    return true;
}
UNITTESTEND(JobManagerNonRecursionTest)
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
    FV_TYPE(TestComponent)

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

    void serialize( TextSerializer& ts ) override { }
    

public:
    Map<String, String> TestMap;
    String s = "Hello TestComponent";
};
FV_TYPE_IMPL(TestComponent)

class TestUpdComponent: public TestComponent
{
    FV_TYPE(TestUpdComponent)
public:
    TestUpdComponent() { }

    ~TestUpdComponent() override { 
        int j = 0;
    }
    void serialize( TextSerializer& ts ) override { }
};
FV_TYPE_IMPL(TestUpdComponent)


UTESTBEGIN(ComponentManagerTest)
{
    for ( u32 j=0; j<10; j++) // See if components are reused on sequential iterations
    {
        u32 k=1000;
        Vector<M<TestComponent>> comps;
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

    List<M<TestUpdComponent>> comps;
    comps.emplace_back( componentManager()->newComponent<TestUpdComponent>() );
    comps.emplace_back( componentManager()->newComponent<TestUpdComponent>() );
    comps.emplace_back( componentManager()->newComponent<TestUpdComponent>() );

    // Should be 3 now
    g = 0;
    for ( TestUpdComponent& tc : Itr<TestUpdComponent>() )
    {
        g++;
    }
    assert( g == 3 );

    // Test when bordering the buffer size of X (eg 128)
    comps.clear();
    List<M<TestComponent>> comps2;
    g = 0;
    for ( u32 i=0; i<1000; i++ )
    {
        auto c = componentManager()->newComponent<TestComponent>();
        comps2.emplace_back( c );
        c->s = "Hello Iterator";
        g++;
    }
    assert( g==1000 && g==componentManager()->numComponents() );

    // Test if mapping to derived component is correct (no invalid ptr offsets)
    for ( auto& tc : Itr<TestComponent>() )
    {
        assert( tc.s == "Hello Iterator" );
    }
    comps2.clear();
    componentManager()->freeAllMemory();

    assert( componentManager()->numComponents<TestComponent>() == 0 );
    assert( componentManager()->numComponents<TestUpdComponent>() == 0 );

    return true;
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

    jobManager()->addJob( addJob2AndWait )->wait();
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
        jobManager()->addJob( addJob1AndWait )->wait();
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
    })->wait();
    else
    {
        M<Job> jChild = jobManager()->addJob([v]() {
         //   printf("Job %d executed \n", v);
            addJob2();
        });

        Suspend(0.0001);
        if ( Random() % 2 == 0 )
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
        jobManager()->addJob(addJob1)->wait();
    }
    return true;
}
UNITTESTEND(JobManagerNonRecursionTest)


UTESTBEGIN(RemoveFromMapWhileIterating)
{
    Map<String, String> map;
    map["Aap"] = "Beer";
    map["Beer"] = "Apple";
    map["Knife"] = "Test";
    map["Knife2"] = "Test2";
    for ( auto it = map.begin(); it != map.end(); )
    {
        printf("Name %s\n", it->first.c_str());
        if ( it->second =="Test" )
        {
            it = map.erase(it);
            continue;
        }
        it++;
    }
    return true;
}
UNITTESTEND(RemoveFromMapWhileIterating)
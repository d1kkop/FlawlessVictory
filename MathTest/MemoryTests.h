#pragma once
#include "UnitTest.h"
#include "../Core.h"
#include "../System/JobManager.h"
#include "../Scene/ComponentManager.h"
#include <cassert>
using namespace fv;

class TestComponent: public Component
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


UTESTBEGIN(JobManagerTest)
{
    Function<void (Job*)> f1, f2;
    //
    //f1 = [=](Job*)
    //{
    //    jobManager()->addJob( f2 )->waitAndFree();
    //};

    //f2 = [=](Job*)
    //{
    //    jobManager()->addJob( f1 )->waitAndFree();
    //};

    //auto lamdaAddJob = [=](Job* j)
    //{
    //    jobManager()->addJob( f1 )->waitAndFree();        
    //};

    jobManager()->addJob( [](Job*)
    {

    })->waitAndFree();
    return true;
}
UNITTESTEND(JobManagerTest)
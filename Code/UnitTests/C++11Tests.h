#pragma once
#include "UnitTest.h"
#include <cassert>
using namespace fv;

static int g_c = 0;

template <typename T>
class MyVector
{
public:
    T* data = nullptr;
    int n = 0;

    void add( const T& e )
    {
        n++;
        T* ndata = (T*)calloc(sizeof(T)*n, 1);
        for( int i =0 ;i < n-1; i++ )
            ndata[i] = data[i];
        ndata[n-1] = e;
        free(data);
        data = ndata;
    }

    MyVector()
    = default;

    MyVector( const std::initializer_list<T>& il )
    {
        for ( auto& e : il )
            add( e );
    }
        
    MyVector( const MyVector& o )
    {
        free(data);
        data = (T*)calloc(sizeof(T)*o.n,1);
        n = o.n;
        for( int i = 0; i < n; i++ )
            data[i] = o.data[i];
    }

    MyVector& operator= ( const MyVector& o )
    {
        free(data);
        data = (T*)calloc(sizeof(T)*o.n,1);
        n = o.n;
        for ( int i = 0; i < n; i++ )
            data[i] = o.data[i];
        return *this;
    }

    MyVector( MyVector&& o )
    {
        data = o.data;
        n = o.n;
        o.data = nullptr;
        o.n = 0;
    }
};

class ByValue
{
public:
    ByValue(int f ):
        va( f )
    {
    }

    ByValue( const ByValue& o ):
        va( o.va )
    {
        g_c++;
    }

    int va = 0;
};

class CppClass
{
public:
    int* pActive = {};
    String k = "11";
    String s = "13";

    CppClass() 
    {
    }

    CppClass(String s):
        k(s)
    {
    }

    CppClass( int* p, const String& k2 ):
        pActive(p),
        k(k2)
    {
    }
};

class CppClass2
{
public:
    int* pActive;
};

UTESTBEGIN(CheckDefaultInitialization)
{
    CppClass cpp;
    CppClass cpp3 = { 0, "12" };
    CppClass cpp4 = {};
    assert( cpp.pActive == nullptr );
    CppClass cpp2("tralala");
    assert( cpp.pActive == nullptr );
    return true;
}
UNITTESTEND( CheckDefaultInitialization )

UTESTBEGIN( CheckContainerCopy )
{
    Map<u32, Vector<ByValue>> m;
    Vector<ByValue> v1 = { 1, 2, 3 };
    Vector<ByValue> v2 = { 10, 20, 30 };
    Vector<ByValue> v3 = { 100, 200, 300 };
    Vector<ByValue> v4 = { 1000, 2000, 3000 };
    Vector<ByValue> v5 = { 10000, 20000, 30000 };
    g_c = 0;
    for ( int i = 0; i < 100; i ++ )
    {
        m.insert( make_pair( 1, v1 ) );
        m.insert( make_pair( 2, v2 ) );
        m.insert( make_pair( 3, v3 ) );
        m.insert( make_pair( 4, v4 ) );
        m.insert( make_pair( 5, v5 ) );
    }
    printf("g_c map (expected 1500) = %d\n", g_c);
    return true;
}
UNITTESTEND( CheckContainerCopy )

UTESTBEGIN( CheckVectorInVectorCopy )
{
    Vector<Vector<ByValue>> v;
    Vector<ByValue> v1 ={ 1, 2, 3 };
    Vector<ByValue> v2 ={ 10, 20, 30 };
    Vector<ByValue> v3 ={ 100, 200, 300 };
    Vector<ByValue> v4 ={ 1000, 2000, 3000 };
    Vector<ByValue> v5 ={ 10000, 20000, 30000 };
    g_c = 0;
    for ( int i = 0; i < 100; i++ )
    {
        v.emplace_back( v1 );
        v.emplace_back( v2 );
        v.emplace_back( v3 );
        v.emplace_back( v4 );
        v.emplace_back( v5 );
    }
    printf( "g_c vector (expected 1500) = %d\n", g_c );
    return true;
}
UNITTESTEND( CheckVectorInVectorCopy )

UTESTBEGIN( CheckMyVectorInVectorCopy )
{
    MyVector<MyVector<ByValue>> v;
    MyVector<ByValue> v1 ={ 1, 2, 3 };
    MyVector<ByValue> v2 ={ 10, 20, 30 };
    MyVector<ByValue> v3 ={ 100, 200, 300 };
    MyVector<ByValue> v4 ={ 1000, 2000, 3000 };
    MyVector<ByValue> v5 ={ 10000, 20000, 30000 };
    g_c = 0;
    for ( int i = 0; i < 100; i++ )
    {
        v.add( v1 );
        v.add( v2 );
        v.add( v3 );
        v.add( v4 );
        v.add( v5 );
    }
    printf( "g_c my vector (expected 0) = %d\n", g_c );
    return true;
}
UNITTESTEND( CheckMyVectorInVectorCopy )
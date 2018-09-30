#pragma once
#include "Common.h"
#include "LogManager.h"
#include <thread>
#include <cassert>

#define FV_CHECK_ST() \
if ( fv::isExecutingParallel() ) \
{ \
    LOGC( "Executing singlethreaded function in parallel. Undefined behaviour!" ); \
    assert( false && "Executing singlethreaded function in parallel. Undefined behaviour!" ); \
}

namespace fv
{
    class Thread: public std::thread
    {
    public:
        Thread() = default;
        Thread(const String& name);
        void setName(const String& name);
    };

    FV_DLL bool isExecutingParallel();
    FV_DLL void setExecutingParallel(bool isParallel);
}

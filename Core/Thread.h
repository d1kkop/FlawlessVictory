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
    using Thread = std::thread;
    FV_DLL bool isExecutingParallel();
    FV_DLL void setExecutingParallel(bool isParallel);
}

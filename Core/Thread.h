#pragma once
#include "Common.h"
#include "LogManager.h"
#include <cassert>

#if FV_DEBUG
    #define FV_CHECK_ST() \
    if ( fv::isExecutingParallel() ) \
    { \
        LOGC( "Executing singlethreaded function in parallel. Undefined behaviour!" ); \
        assert( false && "Executing singlethreaded function in parallel. Undefined behaviour!" ); \
    }
#else
#define FV_CHECK_ST()
#endif

namespace fv
{
    FV_DLL bool isExecutingParallel();
    FV_DLL void setExecutingParallel(bool isParallel);
}

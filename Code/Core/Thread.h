#pragma once
#include "PCH.h"
#include "Common.h"
#include "LogManager.h"

#if FV_DEBUG
    #define FV_CHECK_MO() \
    if ( std::this_thread::get_id() != MainThreadId() ) \
    { \
        LOGC( "Executing singlethreaded function in parallel. Undefined behaviour!" ); \
        assert( false && "Executing singlethreaded function in parallel. Undefined behaviour!" ); \
    }
#else
#define FV_CHECK_MO()
#endif

namespace fv
{
    FV_DLL std::thread::id MainThreadId();
}

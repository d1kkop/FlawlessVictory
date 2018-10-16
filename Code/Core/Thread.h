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
    #define FV_CHECK_BG() \
    if ( !IsBeginFase() ) \
    { \
        LOGC( "Executing read only function after threads were started. Undefined behaviour!" ); \
        assert( false && "Executing read only function after threads were started. Undefined behaviour!" ); \
    }
#else
#define FV_CHECK_MO()
#define FV_CHECK_BG()
#endif

namespace fv
{
    // Engine functions. Do not use.
    FV_DLL std::thread::id MainThreadId();
    FV_DLL bool IsBeginFase();
    FV_DLL void StopBeginFase();
}

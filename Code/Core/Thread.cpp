#include "PCH.h"
#include "Thread.h"

namespace fv
{
    std::thread::id g_MainThreadId = std::this_thread::get_id();

    std::thread::id MainThreadId()
    {
        return g_MainThreadId;
    }
}
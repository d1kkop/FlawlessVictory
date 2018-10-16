#include "PCH.h"
#include "Thread.h"

namespace fv
{
    std::thread::id g_MainThreadId = std::this_thread::get_id();
    bool g_IsBeginFase = true;

    std::thread::id MainThreadId() { return g_MainThreadId; }
    bool IsBeginFase() { return g_IsBeginFase; }
    void StopBeginFase() { g_IsBeginFase = false; }

}
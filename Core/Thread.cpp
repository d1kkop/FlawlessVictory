#include "Thread.h"

namespace fv
{
    bool g_IsExecutingParallel = false;
    FV_DLL bool isExecutingParallel() { return g_IsExecutingParallel; }
    FV_DLL void setExecutingParallel(bool isParallel) { g_IsExecutingParallel=isParallel; }
}
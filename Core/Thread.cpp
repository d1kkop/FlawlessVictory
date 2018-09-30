#include "Thread.h"

namespace fv
{
    Thread::Thread(const String& name)
    {
        setName( name );
    }

    void Thread::setName(const String& name)
    {
    #if FV_INCLUDE_WINHDR
        const DWORD MS_VC_EXCEPTION = 0x406D1388;

    #pragma pack(push,8)  
        struct THREADNAME_INFO
        {
            DWORD dwType; // Must be 0x1000.  
            LPCSTR szName; // Pointer to name (in user addr space).  
            DWORD dwThreadID; // Thread ID (-1=caller thread).  
            DWORD dwFlags; // Reserved for future use, must be zero.  
        };
    #pragma pack(pop)  

        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = name.c_str();
        info.dwThreadID = ::GetCurrentThreadId();
        info.dwFlags = 0;

        #pragma warning(push)  
        #pragma warning(disable: 6320 6322)  
        __try
        {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        }
        __except ( EXCEPTION_EXECUTE_HANDLER ) { }
        #pragma warning(pop)  

        #endif
    }

    bool g_IsExecutingParallel = false;
    FV_DLL bool isExecutingParallel() { return g_IsExecutingParallel; }
    FV_DLL void setExecutingParallel(bool isParallel) { g_IsExecutingParallel=isParallel; }
}
#include "OSLayer.h"
#include "Platform.h"
#include "LogManager.h"
#if FV_SDL
#include "../3rdParty/SDL/include/SDL.h"
#if _MSC_VER
#pragma comment(lib, "../3rdParty/SDL/lib/x64/SDL2.lib")
#endif
#endif

namespace fv
{
    bool OSInitialize()
    {
    #if FV_SDL
        if ( SDL_Init(SDL_INIT_EVERYTHING) != 0 )
        {
            LOGC( "Cannot initialize SDL error %d.", SDL_GetError());
            return false;
        }
    #endif
        return true;
    }

    void OSShutdown()
    {
    #if FV_SDL
        SDL_Quit();
    #endif
    }

    bool OSLoadLibrary(const char* path)
    {
    #if FV_INCLUDE_WINHDR
        HMODULE hModule = ::LoadLibrary(path);
        if ( !hModule )
        {
            LOGC( "Cannot load module %s, error code %d.", path, ::GetLastError() );
            return false;
        }
        return true;
    #endif
        return false;
    }

    FV_DLL void OSSetThreadName(const char* name)
    {
    #if (FV_INCLUDE_WINHDR && _MSC_VER)
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

        // Leak this name
        char* threadName = new char[strlen(name)+1];
        memcpy(threadName, name, strlen(name)+1);

        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = threadName;
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

}
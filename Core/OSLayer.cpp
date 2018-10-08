#include "PCH.h"
#include "OSLayer.h"
#include "Platform.h"
#include "LogManager.h"

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

    void* OSCreateWindow(const char* name, u32 posX, u32 posY, u32 width, u32 height, bool fullscreen,
                         bool supportVulkan, bool supportOpenGL)
    {
    #if FV_SDL
        u32 flags = 0;
        flags |= fullscreen? SDL_WINDOW_FULLSCREEN : 0;
  //      flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        if ( supportVulkan ) flags |= SDL_WINDOW_VULKAN;
        if ( supportOpenGL ) flags |= SDL_WINDOW_OPENGL;
        return SDL_CreateWindow( name, posX, posY, width, height, flags );
    #endif
        return nullptr;
    }

    void OSDestroyWindow(void* pWindow)
    {
    #if FV_SDL
        SDL_DestroyWindow( (SDL_Window*) pWindow );
    #endif
    }


}
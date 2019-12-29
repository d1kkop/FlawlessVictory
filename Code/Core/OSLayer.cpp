#include "PCH.h"
#include "OSLayer.h"
#include "Platform.h"
#include "LogManager.h"
#include "IncWindows.h"
#include "IncSDL.h"
#include "IncGLFW.h"

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
    #elif FV_GLFW
        if (!glfwInit())
        {
            const char* err;
            glfwGetError(&err);
            LOGC( "Cannot initialize glfw error %s.", err );
        }
    #else
        #error no impl
    #endif
        return true;
    }

    void OSShutdown()
    {
    #if FV_SDL
        SDL_Quit();
    #elif FV_GLFW
        glfwTerminate();
    #else
    #error no impl
    #endif
    }

    OSHandle OSLoadLibrary(const char* path)
    {
    #if FV_INCLUDE_WINHDR
        HMODULE hModule = ::LoadLibrary(path);
        if ( !hModule )
        {
            LOGC( "Cannot load module %s, error code %d.", path, ::GetLastError() );
            return {};
        }
        OSHandle h;
        h.set(hModule);
        return h;
    #else
    #error no implementation
    #endif
        return { };
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

    OSHandle OSCreateWindow(const char* name, u32 posX, u32 posY, u32 width, u32 height, bool fullscreen)
    {
        OSHandle h;
    #if FV_SDL
        u32 flags = 0;
        flags |= fullscreen? SDL_WINDOW_FULLSCREEN : 0;
  //      flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        flags |= SDL_WINDOW_VULKAN;
  //      flags |= SDL_WINDOW_OPENGL;
        SDL_Window win = SDL_CreateWindow( name, posX, posY, width, height, flags );
        if ( win ) h.set( win );
    #elif FV_GLFW
        GLFWwindow* win = glfwCreateWindow( width, height, name, fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL );
        if ( win ) h.set( win );
    #else
    #error no impl
    #endif
        return h;
    }

    void OSDestroyWindow(OSHandle handle)
    {
        if ( handle.invalid() ) return;
    #if FV_SDL
        auto ptr = handle.get<SDL_Window*>();
        if ( ptr) SDL_DestroyWindow( ptr );
    #elif FV_GLFW
        auto ptr = handle.get<GLFWwindow*>();
        if ( ptr ) glfwDestroyWindow( ptr );
    #else
    #error no impl
    #endif
    }

    OSHandle OSStartProgram(const char* path, const char* arguments)
    {
    #if FV_INCLUDE_WINHDR
        // https://docs.microsoft.com/en-us/windows/desktop/ProcThread/creating-processes
        STARTUPINFO info = { sizeof(info) };
        PROCESS_INFORMATION processInfo;
        if ( !CreateProcessA(path, (char*)arguments, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &info, &processInfo) )
        {
            return {};
        }
        OSHandle h;
        h.set(processInfo);
        return h;
    #else
    #error no implementation
    #endif
        return {};
    }

    void OSWaitOnProgram(OSHandle handle)
    {
    #if FV_INCLUDE_WINHDR
        static_assert(sizeof( PROCESS_INFORMATION ) <= sizeof( OSHandle ), "OSHandle too small");
        PROCESS_INFORMATION pInfo = handle.get<PROCESS_INFORMATION>();
        WaitForSingleObject( pInfo.hProcess, INFINITE );
        CloseHandle( pInfo.hProcess );
        CloseHandle( pInfo.hThread );
    #else
    #error no implementation
    #endif
    }

    void* OSFindFunction(OSHandle handle, const char* name)
    {
    #if FV_INCLUDE_WINHDR
        HMODULE library = handle.get<HMODULE>();
        return GetProcAddress( library, name );
    #else
    #error no implementation
    #endif
        return { };
    }

}
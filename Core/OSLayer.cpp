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
        HMODULE hModule = ::GetModuleHandle(nullptr);
        if ( !hModule )
        {
            LOGC( "Cannot load module %s, error code %d.", path, ::GetLastError() );
            return false;
        }
    #endif
        return true;
    }
}
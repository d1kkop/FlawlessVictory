#pragma once
#if FV_SDL
    #include "../3rdParty/SDL/include/SDL.h"
    #undef main
    #if _MSC_VER
    #pragma comment(lib, "../3rdParty/SDL/lib/x64/SDL2.lib")
    #endif
#endif
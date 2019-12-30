#pragma once
#if FV_GLFW
    #include "../3rdParty/Glfw/glfw3.h"
#if FV_INCLUDE_WINHDR
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include "../3rdParty/Glfw/glfw3native.h"
#endif
    #if _MSC_VER
    #pragma comment(lib, "../3rdParty/Glfw/glfw3.lib")
    #endif
#endif
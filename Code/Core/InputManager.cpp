#include "PCH.h"
#include "InputManager.h"
#include "InputManagerSDL.h"
#include "Functions.h"

namespace fv
{
    InputManager* g_InputManager {};
    InputManager* inputManager() 
    {
    #if FV_SDL
        auto inputMan = (InputManagerSDL*)g_InputManager;
        g_InputManager = CreateOnce<InputManagerSDL>( inputMan );
        return g_InputManager;
    #else
    #error no implementation
    #endif
        assert(false);
        return nullptr;
    }
    void deleteInputManager() { delete g_InputManager; g_InputManager=nullptr; }
}
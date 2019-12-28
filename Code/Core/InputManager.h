#pragma once
#include "PCH.h"
#include "Common.h"
#include "Input.h"

namespace fv
{
    class InputManager
    {
    public:
        FV_MO FV_DLL virtual bool update() = 0;

        // Keyboard
        FV_MO FV_DLL virtual bool keyPressed( Keyboard key ) = 0;
        FV_MO FV_DLL virtual bool keyDown( Keyboard key ) = 0;
        FV_MO FV_DLL virtual bool keyUp( Keyboard key ) = 0;

        // Mouse
        FV_MO FV_DLL virtual bool mousePressed( Mouse key ) = 0;
        FV_MO FV_DLL virtual bool mouseDown( Mouse key ) = 0;
        FV_MO FV_DLL virtual bool mouseUp( Mouse key ) = 0;
        FV_MO FV_DLL virtual i32 mouseX() = 0;
        FV_MO FV_DLL virtual i32 mouseY() = 0;
        FV_MO FV_DLL virtual i32 mouseScroll() = 0;
        FV_MO FV_DLL virtual i32 mouseDx() = 0;
        FV_MO FV_DLL virtual i32 mouseDy() = 0;
        FV_MO FV_DLL virtual i32 mouseDeltaScroll() = 0;
    };

    FV_DLL InputManager* inputManager();
    FV_DLL void deleteInputManager();
}
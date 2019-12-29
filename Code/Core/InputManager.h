#pragma once
#include "PCH.h"
#include "Common.h"
#include "Input.h"
#include "OSLayer.h"

namespace fv
{
    class InputManager
    {
    public:
        FV_MO FV_DLL virtual bool initialize( OSHandle window ) = 0;
        FV_MO FV_DLL virtual bool update() = 0;

        // Keyboard
        FV_MO FV_DLL virtual bool keyPressed( Keyboard key ) = 0;
        FV_MO FV_DLL virtual bool keyDown( Keyboard key ) = 0;
        FV_MO FV_DLL virtual bool keyReleased( Keyboard key ) = 0;

        // Mouse
        FV_MO FV_DLL virtual bool mousePressed( Mouse key ) = 0;
        FV_MO FV_DLL virtual bool mouseDown( Mouse key ) = 0;
        FV_MO FV_DLL virtual bool mouseReleased( Mouse key ) = 0;
        FV_MO FV_DLL virtual float mouseX() = 0;
        FV_MO FV_DLL virtual float mouseY() = 0;
        FV_MO FV_DLL virtual float mouseDx() = 0;
        FV_MO FV_DLL virtual float mouseDy() = 0;
        FV_MO FV_DLL virtual i32 mouseScroll() = 0;
        FV_MO FV_DLL virtual i32 mouseDeltaScroll() = 0;
    };

    FV_DLL InputManager* inputManager();
    FV_DLL void deleteInputManager();
}
#include "PCH.h"
#include "Input.h"
#include "InputManager.h"

namespace fv
{
    bool KeyDown(Keyboard key)
    {
        return inputManager()->keyDown( key );
    }

    bool KeyPressed(Keyboard key)
    {
        return inputManager()->keyPressed( key );
    }

    bool KeyUp(Keyboard key)
    {
        return inputManager()->keyUp( key );
    }

    bool MouseDown(Mouse key)
    {
        return inputManager()->mouseDown( key );
    }

    bool MousePressed(Mouse key)
    {
        return inputManager()->mousePressed( key );
    }

    bool MouseUp(Mouse key)
    {
        return inputManager()->mouseUp( key );
    }

    i32 MouseX()
    {
        return inputManager()->mouseX();
    }

    i32 MouseY()
    {
        return inputManager()->mouseY();
    }

    i32 MouseScroll()
    {
        return inputManager()->mouseScroll();
    }

    i32 MouseDx()
    {
        return inputManager()->mouseDx();
    }

    i32 MouseDy()
    {
        return inputManager()->mouseDy();
    }

    i32 MouseDeltaScroll()
    {
        return inputManager()->mouseDeltaScroll();
    }

}
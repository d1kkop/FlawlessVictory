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

    bool KeyReleased(Keyboard key)
    {
        return inputManager()->keyReleased( key );
    }

    bool MouseDown(Mouse key)
    {
        return inputManager()->mouseDown( key );
    }

    bool MousePressed(Mouse key)
    {
        return inputManager()->mousePressed( key );
    }

    bool MouseReleased(Mouse key)
    {
        return inputManager()->mouseReleased( key );
    }

    float MouseX()
    {
        return inputManager()->mouseX();
    }

    float MouseY()
    {
        return inputManager()->mouseY();
    }

    float MouseDx()
    {
        return inputManager()->mouseDx();
    }

    float MouseDy()
    {
        return inputManager()->mouseDy();
    }

    i32 MouseScroll()
    {
        return inputManager()->mouseScroll();
    }

    i32 MouseDeltaScroll()
    {
        return inputManager()->mouseDeltaScroll();
    }

}
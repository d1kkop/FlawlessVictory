#include "Input.h"
#include "InputManager.h"

namespace fv
{
    bool Input::keyDown(const char* key)
    {
        return inputManager()->keyDown( key );
    }

    bool Input::keyPressed(const char* key)
    {
        return inputManager()->keyPressed( key );
    }

    bool Input::keyUp(const char* key)
    {
        return inputManager()->keyUp( key );
    }

    bool Input::mouseDown(MouseKey key)
    {
        return inputManager()->mouseDown( key );
    }

    bool Input::mousePressed(MouseKey key)
    {
        return inputManager()->mousePressed( key );
    }

    bool Input::mouseUp(MouseKey key)
    {
        return inputManager()->mouseUp( key );
    }

    i32 Input::mouseX()
    {
        return inputManager()->mouseX();
    }

    i32 Input::mouseY()
    {
        return inputManager()->mouseY();
    }

    i32 Input::mouseScroll()
    {
        return inputManager()->mouseScroll();
    }

    i32 Input::mouseDx()
    {
        return inputManager()->mouseDx();
    }

    i32 Input::mouseDy()
    {
        return inputManager()->mouseDy();
    }

    i32 Input::mouseDeltaScroll()
    {
        return inputManager()->mouseDeltaScroll();
    }

}
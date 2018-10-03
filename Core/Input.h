#pragma once
#include "Common.h"

namespace fv
{
    enum MouseKey
    {
        Left, Right, Middle, X1, X2
    };

    struct FV_DLL Input
    {
        static bool keyDown(const char* key);
        static bool keyPressed(const char* key);
        static bool keyUp(const char* key);

        static bool mouseDown(MouseKey key);
        static bool mousePressed(MouseKey key);
        static bool mouseUp(MouseKey key);

        // Mouse position with respect to window corner.
        static i32 mouseX();
        static i32 mouseY();
        static i32 mouseScroll(); // Mouse scroll value.

        // Mouse change since last input query.
        static i32 mouseDx();
        static i32 mouseDy();
        static i32 mouseDeltaScroll();
    };
}
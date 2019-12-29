#pragma once
#include "PCH.h"
#include "Common.h"

namespace fv
{
    enum class Mouse
    {
        Left, Right, Middle, X1, X2
    };

    enum class Keyboard
    {
        Return,
        Escape,
        Backspace,
        Tab,
        Space,
        Period,
        Slash,
        Key_0,
        Key_1,
        Key_2,
        Key_3,
        Key_4,
        Key_5,
        Key_6,
        Key_7,
        Key_8,
        Key_9,
        SemiColon,
        Equal,
        LeftBracket,
        BackSlash,
        RightBracket,
        BackQuote,
        Key_a,
        Key_b,
        Key_c,
        Key_d,
        Key_e,
        Key_f,
        Key_g,
        Key_h,
        Key_i,
        Key_j,
        Key_k,
        Key_l,
        Key_m,
        Key_n,
        Key_o,
        Key_p,
        Key_q,
        Key_r,
        Key_s,
        Key_t,
        Key_u,
        Key_v,
        Key_w,
        Key_x,
        Key_y,
        Key_z,
        Capslock,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        PrintScreen,
        ScrollLock,
        Pause,
        Insert,
        Home,
        PageUp,
        Delete,
        End,
        PageDown,
        Right,
        Left,
        Down,
        Up
    };

    
    FV_DLL bool KeyDown(Keyboard key);
    FV_DLL bool KeyPressed(Keyboard key);
    FV_DLL bool KeyReleased(Keyboard key);

    FV_DLL bool MouseDown(Mouse key);
    FV_DLL bool MousePressed(Mouse key);
    FV_DLL bool MouseReleased(Mouse key);

    // Mouse position with respect to window corner.
    FV_DLL float MouseX();
    FV_DLL float MouseY();
    FV_DLL i32 MouseScroll(); // Mouse scroll value.

    // Mouse change since last frame.
    FV_DLL float MouseDx();
    FV_DLL float MouseDy();
    FV_DLL i32 MouseDeltaScroll();
}
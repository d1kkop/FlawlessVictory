#pragma once
#include "PCH.h"
#include "Common.h"
#include "Input.h"

namespace fv
{
    class InputManager
    {
    public:
        FV_MO FV_DLL bool update();

        // Keyboard
        FV_MO FV_DLL bool keyPressed(const char* key);
        FV_MO FV_DLL bool keyDown(const char* key);
        FV_MO FV_DLL bool keyUp(const char* key);

        // Mouse
        FV_MO FV_DLL bool mousePressed(MouseKey key);
        FV_MO FV_DLL bool mouseDown(MouseKey key);
        FV_MO FV_DLL bool mouseUp(MouseKey key);
        FV_MO FV_DLL i32 mouseX();
        FV_MO FV_DLL i32 mouseY();
        FV_MO FV_DLL i32 mouseScroll();
        FV_MO FV_DLL i32 mouseDx();
        FV_MO FV_DLL i32 mouseDy();
        FV_MO FV_DLL i32 mouseDeltaScroll();

    private:
        u32 keyNameToScan( const char* key );
        u32 mouseKeyToScan( MouseKey mkey );

        byte m_PrevKeyStates[512]{};
        byte m_CurKeyStates[512]{};
        byte m_PrevMouseKeysState{};
        byte m_CurMouseKeysState{};
        i32 m_CurMouseX{};
        i32 m_CurMouseY{};
        i32 m_CurMouseScroll{};
        i32 m_DeltaMouseX{};
        i32 m_DeltaMouzeY{};
        i32 m_DeltaMouseScroll{};
    };

    FV_DLL InputManager* inputManager();
    FV_DLL void deleteInputManager();
}
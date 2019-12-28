#pragma once
#if FV_SDL
#include "InputManager.h"

namespace fv
{
    class InputManagerSDL : public InputManager
    {
    public:
        FV_MO FV_DLL bool update() override;

        // Keyboard
        FV_MO FV_DLL bool keyPressed(Keyboard key) override;
        FV_MO FV_DLL bool keyDown( Keyboard key) override;
        FV_MO FV_DLL bool keyUp( Keyboard key) override;

        // Mouse
        FV_MO FV_DLL bool mousePressed(Mouse key) override;
        FV_MO FV_DLL bool mouseDown(Mouse key) override;
        FV_MO FV_DLL bool mouseUp(Mouse key) override;
        FV_MO FV_DLL i32 mouseX() override;
        FV_MO FV_DLL i32 mouseY() override;
        FV_MO FV_DLL i32 mouseScroll() override;
        FV_MO FV_DLL i32 mouseDx() override;
        FV_MO FV_DLL i32 mouseDy() override;
        FV_MO FV_DLL i32 mouseDeltaScroll() override;

    private:
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
}
#endif
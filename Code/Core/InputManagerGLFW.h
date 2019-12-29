#pragma once
#if FV_GLFW
#include "IncGLFW.h"
#include "InputManager.h"

namespace fv
{
    class InputManagerGLFW : public InputManager
    {
    public:
        FV_MO FV_DLL bool initialize(OSHandle window);
        FV_MO FV_DLL bool update() override;

        // Keyboard
        FV_MO FV_DLL bool keyPressed(Keyboard key) override;
        FV_MO FV_DLL bool keyDown( Keyboard key) override;
        FV_MO FV_DLL bool keyReleased( Keyboard key) override;

        // Mouse
        FV_MO FV_DLL bool mousePressed(Mouse key) override;
        FV_MO FV_DLL bool mouseDown(Mouse key) override;
        FV_MO FV_DLL bool mouseReleased(Mouse key) override;
        FV_MO FV_DLL float mouseX() override;
        FV_MO FV_DLL float mouseY() override;
        FV_MO FV_DLL float mouseDx() override;
        FV_MO FV_DLL float mouseDy() override;
        FV_MO FV_DLL i32 mouseScroll() override;
        FV_MO FV_DLL i32 mouseDeltaScroll() override;

        void onKeyDown( GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 modes );
        void onMouseDown( GLFWwindow* window, i32 button, i32 action, i32 mods );
        void onMousePos( GLFWwindow* window, double xpos, double ypos );
        void onCursorEnter( GLFWwindow* window, i32 entered );
        void onFocus( GLFWwindow* window, i32 focused );
        void onClose( GLFWwindow* window );

    private:
        void resetInput( GLFWwindow* window );

    private:
        byte m_PrevKeyStates[512]{};
        byte m_CurKeyStates[512]{};
        byte m_PrevMouseKeysState[32]{};
        byte m_CurMouseKeysState[32]{};
        float m_CurMouseX{};
        float m_CurMouseY{};
        float m_DeltaMouseX{};
        float m_DeltaMouseY{};
        i32 m_CurMouseScroll{};
        i32 m_DeltaMouseScroll{};
        bool m_ShouldClose = false;
    };
}
#endif
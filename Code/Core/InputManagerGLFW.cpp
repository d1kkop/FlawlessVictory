#include "PCH.h"
#if FV_GLFW
#include "InputManagerGLFW.h"
#include "Thread.h"
#include "Functions.h"

namespace fv
{
    void glfwOnKeyDown( GLFWwindow* window, int key, int scancode, int action, int modes ) { ((InputManagerGLFW*)inputManager())->onKeyDown( window, key, scancode, action, modes ); }
    void glfwOnMouseDown( GLFWwindow* window, int button, int action, int mods ) { ((InputManagerGLFW*)inputManager())->onMouseDown( window, button, action, mods ); }
    void glfwOnMousePos( GLFWwindow* window, double xpos, double ypos ) { ((InputManagerGLFW*)inputManager())->onMousePos( window, xpos, ypos ); }
    void glfwOnCursorEnter( GLFWwindow* window, int entered ) { ((InputManagerGLFW*)inputManager())->onCursorEnter( window, entered ); }
    void glfwOnFocus( GLFWwindow* window, int focused ) { ((InputManagerGLFW*)inputManager())->onFocus( window, focused ); }
    void glfwOnClose( GLFWwindow* window ) { ((InputManagerGLFW*)inputManager())->onClose( window ); }

    u32 MouseKeyToGlfwKey( Mouse mkey )
    {
        switch ( mkey )
        {
        case Mouse::Left:               return GLFW_MOUSE_BUTTON_LEFT;
        case Mouse::Right:              return GLFW_MOUSE_BUTTON_RIGHT;
        case Mouse::Middle:             return GLFW_MOUSE_BUTTON_MIDDLE;
        case Mouse::X1:                 return GLFW_MOUSE_BUTTON_4;
        case Mouse::X2:                 return GLFW_MOUSE_BUTTON_5;
        }
        return -1;
    }

    u32 KeyboardKeyToGlfwKey( Keyboard key )
    {
        switch ( key )
        {
        case Keyboard::Return:              return GLFW_KEY_ENTER;
        case Keyboard::Escape:              return GLFW_KEY_ESCAPE;
        case Keyboard::Backspace:           return GLFW_KEY_BACKSPACE;
        case Keyboard::Tab:                 return GLFW_KEY_TAB;
        case Keyboard::Space:               return GLFW_KEY_SPACE;
        case Keyboard::Period:              return GLFW_KEY_PERIOD;
        case Keyboard::Slash:               return GLFW_KEY_SLASH;
        case Keyboard::Key_0:               return GLFW_KEY_0;
        case Keyboard::Key_1:               return GLFW_KEY_1;
        case Keyboard::Key_2:               return GLFW_KEY_2;
        case Keyboard::Key_3:               return GLFW_KEY_3;
        case Keyboard::Key_4:               return GLFW_KEY_4;
        case Keyboard::Key_5:               return GLFW_KEY_5;
        case Keyboard::Key_6:               return GLFW_KEY_6;
        case Keyboard::Key_7:               return GLFW_KEY_7;
        case Keyboard::Key_8:               return GLFW_KEY_8;
        case Keyboard::Key_9:               return GLFW_KEY_9;
        case Keyboard::SemiColon:           return GLFW_KEY_SEMICOLON;
        case Keyboard::Equal:               return GLFW_KEY_EQUAL;
        case Keyboard::LeftBracket:         return GLFW_KEY_LEFT_BRACKET;
        case Keyboard::BackSlash:           return GLFW_KEY_BACKSLASH;
        case Keyboard::RightBracket:        return GLFW_KEY_RIGHT_BRACKET;
        case Keyboard::BackQuote:           return GLFW_KEY_GRAVE_ACCENT;
        case Keyboard::Key_a:               return GLFW_KEY_A;
        case Keyboard::Key_b:               return GLFW_KEY_B;
        case Keyboard::Key_c:               return GLFW_KEY_C;
        case Keyboard::Key_d:               return GLFW_KEY_D;
        case Keyboard::Key_e:               return GLFW_KEY_E;
        case Keyboard::Key_f:               return GLFW_KEY_F;
        case Keyboard::Key_g:               return GLFW_KEY_G;
        case Keyboard::Key_h:               return GLFW_KEY_H;
        case Keyboard::Key_i:               return GLFW_KEY_I;
        case Keyboard::Key_j:               return GLFW_KEY_J;
        case Keyboard::Key_k:               return GLFW_KEY_K;
        case Keyboard::Key_l:               return GLFW_KEY_L;
        case Keyboard::Key_m:               return GLFW_KEY_M;
        case Keyboard::Key_n:               return GLFW_KEY_N;
        case Keyboard::Key_o:               return GLFW_KEY_O;
        case Keyboard::Key_p:               return GLFW_KEY_P;
        case Keyboard::Key_q:               return GLFW_KEY_Q;
        case Keyboard::Key_r:               return GLFW_KEY_R;
        case Keyboard::Key_s:               return GLFW_KEY_S;
        case Keyboard::Key_t:               return GLFW_KEY_T;
        case Keyboard::Key_u:               return GLFW_KEY_U;
        case Keyboard::Key_v:               return GLFW_KEY_V;
        case Keyboard::Key_w:               return GLFW_KEY_W;
        case Keyboard::Key_x:               return GLFW_KEY_X;
        case Keyboard::Key_y:               return GLFW_KEY_Y;
        case Keyboard::Key_z:               return GLFW_KEY_Z;
        case Keyboard::Capslock:            return GLFW_KEY_CAPS_LOCK;
        case Keyboard::F1:                  return GLFW_KEY_F1;
        case Keyboard::F2:                  return GLFW_KEY_F2;
        case Keyboard::F3:                  return GLFW_KEY_F3;
        case Keyboard::F4:                  return GLFW_KEY_F4;
        case Keyboard::F5:                  return GLFW_KEY_F5;
        case Keyboard::F6:                  return GLFW_KEY_F6;
        case Keyboard::F7:                  return GLFW_KEY_F7;
        case Keyboard::F8:                  return GLFW_KEY_F8;
        case Keyboard::F9:                  return GLFW_KEY_F9;
        case Keyboard::F10:                 return GLFW_KEY_F10;
        case Keyboard::F11:                 return GLFW_KEY_F11;
        case Keyboard::F12:                 return GLFW_KEY_F12;
        case Keyboard::PrintScreen:         return GLFW_KEY_PRINT_SCREEN;
        case Keyboard::ScrollLock:          return GLFW_KEY_SCROLL_LOCK;
        case Keyboard::Pause:               return GLFW_KEY_PAUSE;
        case Keyboard::Insert:              return GLFW_KEY_INSERT;
        case Keyboard::Home:                return GLFW_KEY_HOME;
        case Keyboard::PageUp:              return GLFW_KEY_PAGE_UP;
        case Keyboard::Delete:              return GLFW_KEY_DELETE;
        case Keyboard::End:                 return GLFW_KEY_END;
        case Keyboard::PageDown:            return GLFW_KEY_PAGE_DOWN;
        case Keyboard::Right:               return GLFW_KEY_RIGHT;
        case Keyboard::Left:                return GLFW_KEY_LEFT;
        case Keyboard::Down:                return GLFW_KEY_DOWN;
        case Keyboard::Up:                  return GLFW_KEY_UP;
        }
        return -1;
    }

    FV_MO bool InputManagerGLFW::initialize(OSHandle handle)
    {
        if ( handle.invalid() )
        {
            LOGC( "Invalid window handle for glfw in input initialization." );
            return false;
        }
        GLFWwindow* window = handle.get<GLFWwindow*>();
        glfwSetKeyCallback( window, glfwOnKeyDown );
        glfwSetMouseButtonCallback( window, glfwOnMouseDown );
        glfwSetCursorPosCallback( window, glfwOnMousePos );
        glfwSetCursorEnterCallback( window, glfwOnCursorEnter );
        glfwSetWindowFocusCallback( window, glfwOnFocus );
        glfwSetWindowCloseCallback( window, glfwOnClose );
        return true;
    }

    FV_MO bool InputManagerGLFW::update()
    {
        FV_CHECK_MO();

        // User clicked close window or something else.
        if ( m_ShouldClose ) 
            return false;

        // First copy current to prev state
        memcpy( m_PrevKeyStates, m_CurKeyStates, sizeof( m_CurKeyStates ) );
        memcpy( m_PrevMouseKeysState, m_CurMouseKeysState, sizeof( m_CurMouseKeysState ) );

        // Update current
        glfwPollEvents();

        return true;
    }

    FV_MO bool InputManagerGLFW::keyPressed(Keyboard key)
    {
        FV_CHECK_MO();
        u32 scan = KeyboardKeyToGlfwKey( key );
        if ( scan >= 0 && scan < sizeof( m_CurKeyStates ) )
        {
            return (!m_PrevKeyStates[scan] && m_CurKeyStates[scan]);
        }
        return false;
    }

    FV_MO bool InputManagerGLFW::keyDown(Keyboard key)
    {
        FV_CHECK_MO();
        u32 scan = KeyboardKeyToGlfwKey(key);
        if ( scan >= 0 && scan < sizeof( m_CurKeyStates ) )
        {
            return (m_CurKeyStates[scan]);
        }
        return false;
    }

    FV_MO bool InputManagerGLFW::keyReleased(Keyboard key)
    {
        FV_CHECK_MO();
        u32 scan = KeyboardKeyToGlfwKey(key);
        if ( scan >= 0 && scan < sizeof(m_CurMouseKeysState) )
        {
            return (m_PrevKeyStates[scan] && !m_CurKeyStates[scan]);
        }
        return false;
    }

    FV_MO bool InputManagerGLFW::mousePressed(Mouse key)
    {
        FV_CHECK_MO();
        u32 scan = MouseKeyToGlfwKey( key );
        if ( scan >= 0 && scan < sizeof(m_CurMouseKeysState) )
        {
            return (!m_PrevMouseKeysState[scan] && m_CurMouseKeysState[scan]);
        }
        return false;
    }

    FV_MO bool InputManagerGLFW::mouseDown(Mouse key)
    {
        FV_CHECK_MO();
        u32 scan = MouseKeyToGlfwKey(key);
        if ( scan >= 0 && scan < sizeof( m_CurMouseKeysState ) )
        {
            return m_CurMouseKeysState[scan];
        }
        return false;
    }

    FV_MO bool InputManagerGLFW::mouseReleased(Mouse key)
    {
        FV_CHECK_MO();
        u32 scan = MouseKeyToGlfwKey(key);
        if ( scan >= 0 && scan < sizeof( m_CurMouseKeysState ) )
        {
            return (m_PrevMouseKeysState[scan] && !m_CurMouseKeysState[scan]);
        }
        return false;
    }

    FV_MO float InputManagerGLFW::mouseX()
    {
        FV_CHECK_MO();
        return m_CurMouseX;
    }

    FV_MO float InputManagerGLFW::mouseY()
    {
        FV_CHECK_MO();
        return m_CurMouseY;
    }

    FV_MO float InputManagerGLFW::mouseDx()
    {
        FV_CHECK_MO();
        return m_DeltaMouseX;
    }

    FV_MO float InputManagerGLFW::mouseDy()
    {
        FV_CHECK_MO();
        return m_DeltaMouseY;
    }

    FV_MO i32 InputManagerGLFW::mouseScroll()
    {
        FV_CHECK_MO();
        return m_CurMouseScroll;
    }

    FV_MO i32 InputManagerGLFW::mouseDeltaScroll()
    {
        FV_CHECK_MO();
        return m_DeltaMouseScroll;
    }

    void InputManagerGLFW::onKeyDown( GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 modes )
    {
        if ( key >= sizeof(m_CurKeyStates) )
        {
            LOGW("Invalid key %s detected, cannot put in array.", key );
            return;
        }
        if ( action == GLFW_PRESS )
            m_CurKeyStates[key] = true;
        else if (action == GLFW_RELEASE)
            m_CurKeyStates[key] = false;
    }

    
    void InputManagerGLFW::onMouseDown( GLFWwindow* window, i32 button, i32 action, i32 mods )
    {
        if ( button >= sizeof( m_CurMouseKeysState ) )
        {
            LOGW( "Invalid key %s detected, cannot put in array.", button );
            return;
        }
        if ( action == GLFW_PRESS )
            m_CurMouseKeysState[button] = true;
        else if ( action == GLFW_RELEASE )
            m_CurMouseKeysState[button] = false;
    }

    void InputManagerGLFW::onMousePos( GLFWwindow* window, double xpos, double ypos )
    {
        m_DeltaMouseX = (float)xpos - m_CurMouseX;
        m_DeltaMouseY = (float)ypos - m_CurMouseY;
        m_CurMouseX = (float)xpos;
        m_CurMouseY = (float)ypos;
    }

    void InputManagerGLFW::onCursorEnter( GLFWwindow* window, i32 entered )
    {
        resetInput( window );
    }

    void InputManagerGLFW::onFocus( GLFWwindow* window, i32 focused )
    {
        resetInput( window );
    }

    void InputManagerGLFW::onClose( GLFWwindow* window )
    {
        m_ShouldClose = true;
    }

    void InputManagerGLFW::resetInput( GLFWwindow* window )
    {
        // Sync curmouse x, y and dx, dy to avoid sudden big delta
        double x, y;
        glfwGetCursorPos( window, &x, &y );
        m_CurMouseX = (float)x;
        m_CurMouseY = (float)y;
        m_DeltaMouseX = m_DeltaMouseY = 0;
        // Reset states
        memset( m_CurKeyStates, 0, sizeof( m_CurKeyStates ) );
        memset( m_PrevKeyStates, 0, sizeof( m_PrevKeyStates ) );
        memset( m_CurMouseKeysState, 0, sizeof( m_CurMouseKeysState ) );
        memset( m_CurMouseKeysState, 0, sizeof( m_CurMouseKeysState ) );
    }

}
#endif
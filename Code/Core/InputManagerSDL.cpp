#include "PCH.h"
#include "InputManagerSDL.h"
#if FV_SDL
#include "IncSDL.h"
#include "Thread.h"

namespace fv
{
    u32 SDL_MouseKeyToScan( Mouse mkey )
    {
        switch ( mkey )
        {
        case Mouse::Left:               return SDL_BUTTON( SDL_BUTTON_LEFT );
        case Mouse::Right:              return SDL_BUTTON( SDL_BUTTON_RIGHT );
        case Mouse::Middle:             return SDL_BUTTON( SDL_BUTTON_MIDDLE );
        case Mouse::X1:                 return SDL_BUTTON( SDL_BUTTON_X1 );
        case Mouse::X2:                 return SDL_BUTTON( SDL_BUTTON_X2 );
        }
        return -1;
    }

    u32 SDL_KeyboardKeyToScan( Keyboard key )
    {
        switch ( key )
        {
        case Keyboard::Return:              return SDLK_RETURN;
        case Keyboard::Escape:              return SDLK_ESCAPE;
        case Keyboard::Backspace:           return SDLK_BACKSPACE;
        case Keyboard::Tab:                 return SDLK_TAB;
        case Keyboard::Space:               return SDLK_SPACE;
        case Keyboard::Exclaim:             return SDLK_EXCLAIM;
        case Keyboard::QuoteDbl:            return SDLK_QUOTEDBL;
        case Keyboard::Hash:                return SDLK_HASH;
        case Keyboard::Percent:             return SDLK_PERCENT;
        case Keyboard::Dollar:              return SDLK_DOLLAR;
        case Keyboard::Ampersand:           return SDLK_AMPERSAND;
        case Keyboard::Quote:               return SDLK_QUOTE;
        case Keyboard::LeftParenthesis:     return SDLK_LEFTPAREN;
        case Keyboard::RightParenthesis:    return SDLK_RIGHTPAREN;
        case Keyboard::Asterisk:            return SDLK_ASTERISK;
        case Keyboard::Plus:                return SDLK_PLUS;
        case Keyboard::Comma:               return SDLK_COMMA;
        case Keyboard::Minus:               return SDLK_MINUS;
        case Keyboard::Period:              return SDLK_PERIOD;
        case Keyboard::Slash:               return SDLK_SLASH;
        case Keyboard::Key_0:               return SDLK_0;
        case Keyboard::Key_1:               return SDLK_1;
        case Keyboard::Key_2:               return SDLK_2;
        case Keyboard::Key_3:               return SDLK_3;
        case Keyboard::Key_4:               return SDLK_4;
        case Keyboard::Key_5:               return SDLK_5;
        case Keyboard::Key_6:               return SDLK_6;
        case Keyboard::Key_7:               return SDLK_7;
        case Keyboard::Key_8:               return SDLK_8;
        case Keyboard::Key_9:               return SDLK_9;
        case Keyboard::Colon:               return SDLK_COLON;
        case Keyboard::SemiColon:           return SDLK_SEMICOLON;
        case Keyboard::Less:                return SDLK_LESS;
        case Keyboard::Equal:               return SDLK_EQUALS;
        case Keyboard::Greater:             return SDLK_GREATER;
        case Keyboard::QuestionMark:        return SDLK_QUESTION;
        case Keyboard::At:                  return SDLK_AT;
        case Keyboard::LeftBracket:         return SDLK_LEFTBRACKET;
        case Keyboard::BackSlash:           return SDLK_BACKSLASH;
        case Keyboard::RightBracket:        return SDLK_RIGHTBRACKET;
        case Keyboard::Caret:               return SDLK_CARET;
        case Keyboard::Underscore:          return SDLK_UNDERSCORE;
        case Keyboard::BackQuote:           return SDLK_BACKQUOTE;
        case Keyboard::Key_a:               return SDLK_a;
        case Keyboard::Key_b:               return SDLK_b;
        case Keyboard::Key_c:               return SDLK_c;
        case Keyboard::Key_d:               return SDLK_d;
        case Keyboard::Key_e:               return SDLK_e;
        case Keyboard::Key_f:               return SDLK_f;
        case Keyboard::Key_g:               return SDLK_g;
        case Keyboard::Key_h:               return SDLK_h;
        case Keyboard::Key_i:               return SDLK_i;
        case Keyboard::Key_j:               return SDLK_j;
        case Keyboard::Key_k:               return SDLK_k;
        case Keyboard::Key_l:               return SDLK_l;
        case Keyboard::Key_m:               return SDLK_m;
        case Keyboard::Key_n:               return SDLK_n;
        case Keyboard::Key_o:               return SDLK_o;
        case Keyboard::Key_p:               return SDLK_p;
        case Keyboard::Key_q:               return SDLK_q;
        case Keyboard::Key_r:               return SDLK_r;
        case Keyboard::Key_s:               return SDLK_s;
        case Keyboard::Key_t:               return SDLK_t;
        case Keyboard::Key_u:               return SDLK_u;
        case Keyboard::Key_v:               return SDLK_v;
        case Keyboard::Key_w:               return SDLK_w;
        case Keyboard::Key_x:               return SDLK_x;
        case Keyboard::Key_y:               return SDLK_y;
        case Keyboard::Key_z:               return SDLK_z;
        case Keyboard::Capslock:            return SDLK_CAPSLOCK;
        case Keyboard::F1:                  return SDLK_F1;
        case Keyboard::F2:                  return SDLK_F2;
        case Keyboard::F3:                  return SDLK_F3;
        case Keyboard::F4:                  return SDLK_F4;
        case Keyboard::F5:                  return SDLK_F5;
        case Keyboard::F6:                  return SDLK_F6;
        case Keyboard::F7:                  return SDLK_F7;
        case Keyboard::F8:                  return SDLK_F8;
        case Keyboard::F9:                  return SDLK_F9;
        case Keyboard::F10:                 return SDLK_F10;
        case Keyboard::F11:                 return SDLK_F11;
        case Keyboard::F12:                 return SDLK_F12;
        case Keyboard::PrintScreen:         return SDLK_PRINTSCREEN;
        case Keyboard::ScrollLock:          return SDLK_SCROLLLOCK;
        case Keyboard::Pause:               return SDLK_PAUSE;
        case Keyboard::Insert:              return SDLK_INSERT;
        case Keyboard::Home:                return SDLK_HOME;
        case Keyboard::PageUp:              return SDLK_PAGEUP;
        case Keyboard::Delete:              return SDLK_DELETE;
        case Keyboard::End:                 return SDLK_END;
        case Keyboard::PageDown:            return SDLK_PAGEDOWN;
        case Keyboard::Right:               return SDLK_RIGHT;
        case Keyboard::Left:                return SDLK_LEFT;
        case Keyboard::Down:                return SDLK_DOWN;
        case Keyboard::Up:                  return SDLK_UP;
        }

        return -1;
    }

    FV_MO bool InputManagerSDL::update()
    {
        FV_CHECK_MO();

        //SDL_Event event;
        //while ( SDL_PollEvent(&event) )
        //{
        //    if ( event.type == SDL_QUIT )
        //        return false;
        //}
        SDL_PumpEvents();

        // Keyboard
        {
            memcpy( m_PrevKeyStates, m_CurKeyStates, sizeof(m_CurKeyStates) );
            int numKeys = 0;
            const byte* sdlKeys = SDL_GetKeyboardState(&numKeys);
            assert( numKeys <= 512 ); // Size of state array
            memcpy( m_CurKeyStates, sdlKeys, numKeys*sizeof(byte) );
        }

        // Mouse
        {
            i32 prevMouseX = m_CurMouseX;
            i32 prevMouseY = m_CurMouseY;
            m_PrevMouseKeysState = m_CurMouseKeysState;
            m_CurMouseKeysState  = SDL_GetMouseState( (int*)&m_CurMouseX, (int*)&m_CurMouseY );
        }

        return true;
    }

    FV_MO bool InputManagerSDL::keyPressed(Keyboard key)
    {
        FV_CHECK_MO();
        u32 scan = SDL_KeyboardKeyToScan( key );
        if ( scan != -1 )
        {
            return (!m_PrevKeyStates[scan] && m_CurKeyStates[scan]);
        }
        return false;
    }

    FV_MO bool InputManagerSDL::keyDown(Keyboard key)
    {
        FV_CHECK_MO();
        u32 scan = SDL_KeyboardKeyToScan(key);
        if ( scan != -1 )
        {
            return (m_CurKeyStates[scan]);
        }
        return false;
    }

    FV_MO bool InputManagerSDL::keyUp(Keyboard key)
    {
        FV_CHECK_MO();
        u32 scan = SDL_KeyboardKeyToScan(key);
        if ( scan != -1 )
        {
            return (!m_CurKeyStates[scan]);
        }
        return false;
    }

    FV_MO bool InputManagerSDL::mousePressed(Mouse key)
    {
        FV_CHECK_MO();
        u32 scan = SDL_MouseKeyToScan( key );
        if ( scan != -1 )
        {
            return ((m_PrevMouseKeysState & scan)==0 && (m_CurMouseKeysState & scan)!=0);
        }
        return false;
    }

    FV_MO bool InputManagerSDL::mouseDown(Mouse key)
    {
        FV_CHECK_MO();
        u32 scan = SDL_MouseKeyToScan(key);
        if ( scan != -1 )
        {
            return ((m_CurMouseKeysState & scan)!=0);
        }
        return false;
    }

    FV_MO bool InputManagerSDL::mouseUp(Mouse key)
    {
        FV_CHECK_MO();
        u32 scan = SDL_MouseKeyToScan(key);
        if ( scan != -1 )
        {
            return ((m_CurMouseKeysState & scan)==0);
        }
        return false;
    }

    FV_MO i32 InputManagerSDL::mouseX()
    {
        FV_CHECK_MO();
        return m_CurMouseX;
    }

    FV_MO i32 InputManagerSDL::mouseY()
    {
        FV_CHECK_MO();
        return m_CurMouseY;
    }

    FV_MO i32 InputManagerSDL::mouseScroll()
    {
        FV_CHECK_MO();
        return m_CurMouseScroll;
    }

    FV_MO i32 InputManagerSDL::mouseDx()
    {
        FV_CHECK_MO();
        return m_DeltaMouseX;
    }

    FV_MO i32 InputManagerSDL::mouseDy()
    {
        FV_CHECK_MO();
        return m_DeltaMouzeY;
    }

    FV_MO i32 InputManagerSDL::mouseDeltaScroll()
    {
        FV_CHECK_MO();
        return m_DeltaMouseScroll;
    }
}
#endif
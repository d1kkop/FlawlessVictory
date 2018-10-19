#include "PCH.h"
#include "InputManager.h"
#include "Functions.h"
#include "Thread.h"
#include <cassert>
#include "IncSDL.h"

namespace fv
{
    bool InputManager::update()
    {
        FV_CHECK_MO();

    #if FV_SDL
        SDL_Event event;
        while ( SDL_PollEvent(&event) )
        {
            if ( event.type == SDL_QUIT )
                return false;
        }
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
    #endif
        return true;
    }


    bool InputManager::keyPressed(const char* key)
    {
        FV_CHECK_MO();
        u32 scan = keyNameToScan( key );
        if ( scan != -1 )
        {
            return (!m_PrevKeyStates[scan] && m_CurKeyStates[scan]);
        }
        return false;
    }

    bool InputManager::keyDown(const char* key)
    {
        FV_CHECK_MO();
        u32 scan = keyNameToScan(key);
        if ( scan != -1 )
        {
            return (m_CurKeyStates[scan]);
        }
        return false;
    }

    bool InputManager::keyUp(const char* key)
    {
        FV_CHECK_MO();
        u32 scan = keyNameToScan(key);
        if ( scan != -1 )
        {
            return (!m_CurKeyStates[scan]);
        }
        return false;
    }

    bool InputManager::mousePressed(MouseKey key)
    {
        FV_CHECK_MO();
        u32 scan = mouseKeyToScan( key );
        if ( scan != -1 )
        {
            return ((m_PrevMouseKeysState & scan)==0 && (m_CurMouseKeysState & scan)!=0);
        }
        return false;
    }

    bool InputManager::mouseDown(MouseKey key)
    {
        FV_CHECK_MO();
        u32 scan = mouseKeyToScan(key);
        if ( scan != -1 )
        {
            return ((m_CurMouseKeysState & scan)!=0);
        }
        return false;
    }

    bool InputManager::mouseUp(MouseKey key)
    {
        FV_CHECK_MO();
        u32 scan = mouseKeyToScan(key);
        if ( scan != -1 )
        {
            return ((m_CurMouseKeysState & scan)==0);
        }
        return false;
    }

    i32 InputManager::mouseX()
    {
        FV_CHECK_MO();
        return m_CurMouseX;
    }

    i32 InputManager::mouseY()
    {
        FV_CHECK_MO();
        return m_CurMouseY;
    }

    i32 InputManager::mouseScroll()
    {
        FV_CHECK_MO();
        return m_CurMouseScroll;
    }

    i32 InputManager::mouseDx()
    {
        FV_CHECK_MO();
        return m_DeltaMouseX;
    }

    i32 InputManager::mouseDy()
    {
        FV_CHECK_MO();
        return m_DeltaMouzeY;
    }

    i32 InputManager::mouseDeltaScroll()
    {
        FV_CHECK_MO();
        return m_DeltaMouseScroll;
    }

    u32 InputManager::keyNameToScan(const char* key)
    {
    #if FV_SDL
        SDL_Keycode sdlCode = SDL_GetKeyFromName(key);
        if ( sdlCode != SDLK_UNKNOWN )
        {
            u32 scan = SDL_GetScancodeFromKey(sdlCode);
            assert( scan < 512 ); // KeyCode buffer size
            return scan;
        }
    #endif
        return (u32)-1;
    }

    u32 InputManager::mouseKeyToScan(MouseKey mkey)
    {
    #if FV_SDL
        switch ( mkey )
        {
        case MouseKey::Left:
            return SDL_BUTTON(SDL_BUTTON_LEFT);
        case MouseKey::Right:
            return SDL_BUTTON(SDL_BUTTON_RIGHT);
        case MouseKey::Middle:
            return SDL_BUTTON(SDL_BUTTON_MIDDLE);
        case MouseKey::X1:
            return SDL_BUTTON(SDL_BUTTON_X1);
        case MouseKey::X2:
            return SDL_BUTTON(SDL_BUTTON_X2);
        }
    #endif
        return (u32)-1;
    }

    InputManager* g_InputManager {};
    InputManager* inputManager() { return CreateOnce(g_InputManager); }
    void deleteInputManager() { delete g_InputManager; g_InputManager=nullptr; }
}
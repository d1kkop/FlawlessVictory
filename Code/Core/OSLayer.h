#pragma once
#include "Common.h"

namespace fv
{
    struct OSHandle
    {
    public:
        template <typename T>
        void set(T d)
        { 
            static_assert( sizeof(m_Data) >= sizeof(T), "OSHandle too small." );
            memcpy(m_Data, &d, sizeof(d)); 
            m_Invalid = false;
        }

        template <typename T>
        T get()
        {
            static_assert( sizeof( m_Data ) >= sizeof( T ), "OSHandle too small.");
            T d;
            memcpy(&d, m_Data, sizeof(T));
            return d;
        }

        void setZero()
        {
            memset(m_Data, 0, sizeof(m_Data));
            m_Invalid = true;
        }

        bool invalid() const { return m_Invalid; }

    private:
        char m_Data[128]{};
        bool m_Invalid = true;
    };

    FV_DLL bool OSInitialize();
    FV_DLL void OSShutdown();
    FV_DLL OSHandle OSLoadLibrary(const char* path);
    FV_DLL void OSSetThreadName(const char* name);
    FV_DLL OSHandle OSCreateWindow(const char* name, u32 posX, u32 posY, u32 width, u32 height, bool fullscreen);
    FV_DLL void OSDestroyWindow(OSHandle handle);
    FV_DLL OSHandle OSStartProgram(const char* path, const char* arguments);
    FV_DLL void OSWaitOnProgram(OSHandle handle);
    FV_DLL void* OSFindFunction(OSHandle handle, const char* name);
}
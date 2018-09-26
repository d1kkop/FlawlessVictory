#pragma once
#include "Platform.h"
#include "Common.h"


namespace fv
{
    class Object;

    class ObjectManager
    {
        const u32 ObjectBufferSize = 128;

    public:
        ObjectManager();
        FV_DLL Object* newObject();

    private:
        void growObjects();
        Set<Object*> m_ActiveObjects;
        Array<Object*> m_FreeObjects;
    };
}
#pragma once
#include "Common.h"
#include "Component.h"

namespace fv
{
    /*  The ComponentManager serves two purposes.
        1. All components are supposed to exist at all time. Freed components are recycled.
        2. Operations can be performed on a sequential array of memory (roughly) in a ordered fashion. This is very cache friendly. */
    class ComponentManager
    {
        const u32 ComponentBufferSize = 64;

    public:
        ComponentManager() = default;
        ~ComponentManager();

        FV_DLL Component* newComponent(u32 type);
        FV_DLL void growComponents(u32 type);
        FV_DLL Map<u32, Array<Component*>>& components();
        FV_DLL void freeComponent(Component* c);

        template <class T> T* newComponent();

    private:
        bool validType(u32 type);

        Map<u32, Array<Component*>> m_Components;
        Map<u32, Array<Component*>> m_FreeComponents;
    };

    template <class T>
    T* ComponentManager::newComponent()
    {
        T* t = sc<T*>(newComponent(T::type()));
        return t;
    }
}

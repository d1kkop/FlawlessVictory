#pragma once
#include "PCH.h"
#include "Common.h"

namespace fv
{
    class Destructable
    {
    };

    // The purpose of this manager is to pospone destruction of objects to a different thread.
    // Eg, components created on the main thread or in the render thread can register objects at the destruction manager upon creation.
    // Then, when the reference count becomes 1, the destruction manager removes the item from the list in a seperate thread (destruction thread).
    // The benefit is destruction is deferred and therefore cannot cause hickups on main or other threads.
    // Another benefit is that the destruction of objects is in a single place, so you keep control of where objects are eventually destructed.
    class DestructionManager
    {
    public:
        FV_DLL DestructionManager();
        FV_DLL ~DestructionManager();

        template <typename T>
        FV_TS void add( const M<T>& destructable );

        // Adds a destructable.
        FV_TS FV_DLL void add( const M<Destructable>& destructable );

        // Gets rid of all registered destructables.
        FV_TS FV_DLL void clearListOfDestructables();

    private:
        void ThreadLoop();

    private:
        Thread m_DestructionThread;
        List<M<Destructable>> m_Destructables;
        Mutex m_ListMutex;
        Atomic<bool> m_Closing;
    };


    FV_DLL DestructionManager* destructionManager();
    FV_DLL void deleteDestructionManager();

    template <typename T>
    FV_TS void fv::DestructionManager::add( const M<T>& destructable )
    {
        M<Destructable> d = spc<Destructable>( destructable );
        add( d );
    }

}

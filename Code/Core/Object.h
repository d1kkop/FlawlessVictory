#pragma once
#include "PCH.h"
#include "Common.h"

namespace fv
{
    // ------------ Object ----------------------------------------------------------------------------------------

    class Object
    {
    public:
        bool inUse() const { return m_Active; }
        u32 version() const { return m_Version; }

    private:
        u32 m_Version = 0;
        bool m_Freed  = false;
        bool m_Active = false;

        template <class T>
        friend class ObjectManager;
        friend class ComponentManager;
        friend class ComponentManager2;
    };

    // ------------ ObjectArray ----------------------------------------------------------------------------------------

    struct ObjectArray
    {
        Object* elements;
        u32 size;
    };

    // ------------ Ref to recycled Object -------------------------------------------------------------------------------
    // NOT thread safe.

    template <class T>
    class Ref
    {
    public:
        Ref() = default;
        Ref(T* obj): m_Object(obj)
        {
            if ( obj ) m_CachedVersion = obj->version();
        }
        // Default asignment, move and copy constructor

        T* operator->() const { return (*this?m_Object:nullptr); }
        operator bool() const { return m_Object && m_Object->version()==m_CachedVersion; }

    private:
        T* m_Object{};
        u32 m_CachedVersion = 0;
    };
}
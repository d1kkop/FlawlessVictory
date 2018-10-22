#pragma once
#include "PCH.h"
#include "Common.h"

namespace fv
{
    struct Type;

    using CreateFunc = Type* (*)(u32);
    using ResetFunc  = void (*)(Type*);

    struct FV_DLL Type
    {
        u32 type() const { return m_Type; }

    private:
        u32 m_Type = -1;

        friend class TypeManager;
    };

    struct FV_DLL TypeInfo
    {
        u32 hash;
        u32 size;
        CreateFunc createFunc;
        ResetFunc resetFunc;
        const String* name;
        u32 flags;
    };

    /*  Retrieving type info from typemanager is thread safe as it is initialized upon static object initialization. No further changes
        are then made to the registered types anymore. 
        The TypeManager is designed for unknown types that are to be created in different user DLL's. As such, the type can be mapped from 
        integer to a static create and placement new operator.
        Using an enum would not suffice as the types need to be known beforehand. */
    class TypeManager
    {
    public:
        FV_DLL u32 registerType(const char* typeName, u32 size, CreateFunc cfunc, ResetFunc rfunc, u32 flags);
        FV_DLL const TypeInfo* typeInfo(const char* typeName);
        FV_DLL const TypeInfo* typeInfo(u32 hash);
        FV_DLL Type* createTypes(u32 type, u32 num);
        FV_DLL Type* createTypes(const TypeInfo&, u32 num);

        static void setType( u32 type, Type& t );

    private:
        Map<String, TypeInfo> m_NameToType;
        Map<u32, TypeInfo*> m_HashToType;
    };

    FV_DLL TypeManager* typeManager();
    FV_DLL void deleteTypeManager();
}
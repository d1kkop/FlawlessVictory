#pragma once
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
        i32 updatePriority;
        char extension[FV_MAX_EXTENSION];
    };

    class TypeManager
    {
    public:
        FV_DLL u32 registerType(const char* typeName, u32 size, CreateFunc cfunc, ResetFunc rfunc, i32 updatePriority, const char* extension);
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
#pragma once
#include "Common.h"

namespace fv
{
    class Component;

    using CreateFunc = Component* (*)(u32);
    using ResetFunc  = void (*)(Component*);


    struct FV_DLL TypeInfo
    {
        u32 hash;
        u32 size;
        CreateFunc createFunc;
        ResetFunc resetFunc;
        const String* name;
    };

    class TypeManager
    {
    public:
        FV_DLL u32 registerType(const char* typeName, u32 size, CreateFunc cfunc, ResetFunc rfunc);
        FV_DLL const TypeInfo& typeInfo(const char* typeName);
        FV_DLL const TypeInfo& typeInfo(u32 hash);

    private:
        Map<String, TypeInfo> m_NameToType;
        Map<u32, TypeInfo*> m_HashToType;
    };

    FV_DLL TypeManager* typeManager();
    FV_DLL void deleteTypeManager();
}
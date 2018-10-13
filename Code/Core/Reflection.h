#pragma once
#include "TypeManager.h"

#define FV_TYPE_COMMON(name) \
    public:\
    static FV_DLL u32 type() { auto* t=fv::typeManager()->typeInfo(#name); return t?t->hash:(-1); } \
    static FV_DLL Type* create(u32 num) { return num==1?(new name):(new name[num]); } /* Support single allocation for shared ptrs */ \
    static FV_DLL void reset(Type* c) { new (sc<name*>(c))name; }

#define FV_TYPE(name) \
    FV_TYPE_COMMON(name) \
    static FV_DLL u32 registerType() { return fv::typeManager()->registerType(#name, sizeof(name), name::create, name::reset); }

#define FV_RESOURCE_TYPE(name) \
    FV_TYPE(name)

#define FV_TYPE_IMPL(name) \
static u32 name##_Initializer = name::registerType();
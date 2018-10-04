#pragma once
#include "TypeManager.h"

#define FV_TYPE_COMMON(name) \
    public:\
    static FV_DLL u32 type() { auto* t=fv::typeManager()->typeInfo(#name); return t?t->hash:(-1); } \
    static FV_DLL Type* create(u32 num) { return (new name[num]); } \
    static FV_DLL void reset(Type* c) { new (c)name; }

#define FV_TYPE(name, upd_priority, exec_in_parallel) \
    FV_TYPE_COMMON(name) \
    static FV_DLL u32 registerType() { return fv::typeManager()->registerType(#name, sizeof(name), name::create, name::reset, (upd_priority), ""); }

#define FV_RESOURCE_TYPE(name, extension) \
    FV_TYPE_COMMON(name) \
    static FV_DLL u32 registerType() { return fv::typeManager()->registerType(#name, sizeof(name), name::create, name::reset, 0, ""); }

#define FV_TYPE_IMPL(name) \
static u32 name##_Initializer = name::registerType();

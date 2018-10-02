#pragma once
#include "TypeManager.h"

#define FV_TYPE(name, upd_priority, exec_in_parallel) \
public:\
u32 updatePriority() const override { return (upd_priority); } \
static FV_DLL u32 type() { return fv::typeManager()->typeInfo(#name).hash; } \
static FV_DLL Component* create(u32 num) { return (new name[num]); } \
static FV_DLL void reset(Component* c) { new (c)name; } \
static FV_DLL u32 registerType() { return fv::typeManager()->registerType(#name, sizeof(name), name::create, name::reset); }

#define FV_TYPE_IMPL(name) \
static u32 name##_Initializer = name::registerType();

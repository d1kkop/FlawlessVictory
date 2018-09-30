#pragma once
#include "CoreMain.h"
#include "TypeManager.h"

#define FV_TYPE(name, upd_priority, exec_in_parallel) \
public:\
u32 updatePriority() const override { return (upd_priority); } \
static FV_DLL u32 type() { return typeManager()->typeInfo(#name).hash; } \
static FV_DLL Component* create(u32 num) { return sc<Component*>(new name[num]); } \
static FV_DLL void reset(Component* c) { new (c)name; } \
static FV_DLL u32 registerType() { return typeManager()->registerType(#name, name::create, name::reset); }

#define FV_TYPE_IMPL(name) \
static u32 name##_Initializer = name::registerType();

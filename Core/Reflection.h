#pragma once
#include "CoreMain.h"
#include "TypeManager.h"

#define FV_TYPE(name, upd_priority, exec_in_parallel) \
public:\
u32 updatePriority() const override { return (upd_priority); } \
static FV_DLL u32 type() { return typeManager()->nameToType(#name); } \
static FV_DLL bool execParallel() { return (exec_in_parallel); }
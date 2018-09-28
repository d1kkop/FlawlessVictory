#pragma once
#include "CoreManagers.h"
#include "TypeManager.h"

#define FV_TYPE(name) \
public:\
static u32 type() \
{ \
    return fv::typeManager()->nameToType(#name); \
}
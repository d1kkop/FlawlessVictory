#pragma once
#include "TypeManager.h"
#include "System.h"

#define FV_TYPE(name) \
public:\
static u32 type() \
{ \
    return fv::typeManager()->nameToType(#name); \
}
#pragma once
#include "Platform.h"

namespace fv
{
    FV_DLL void startup();
    FV_DLL void shutdown();


    FV_DLL class ObjectManager* objectManager();
    FV_DLL class ComponentManager* componentManager();
    FV_DLL class TypeManager* typeManager();
}

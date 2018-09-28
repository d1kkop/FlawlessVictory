#pragma once
#include "Platform.h"
#include "ObjectManager.h"

namespace fv
{
    class GameObject;

    FV_DLL void startup();
    FV_DLL void shutdown();


    FV_DLL class ObjectManager<GameObject>* gameObjectManager();
    FV_DLL class ComponentManager* componentManager();
    FV_DLL class TypeManager* typeManager();
}

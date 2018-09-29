#pragma once
#include "../Core/ObjectManager.h"
#include "GameObject.h"

namespace fv
{
    FV_DLL void scene_start();
    FV_DLL void scene_close();
    FV_DLL ObjectManager<GameObject>* gameObjectManager();
}
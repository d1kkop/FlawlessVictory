#pragma once
#include "../Core/ObjectManager.h"
#include "GameObject.h"

#define FV_SCENECOMPONENT_PRIORITY (-1000)
#define FV_CAMERA_PRIORITY (-990)

namespace fv
{
    FV_DLL void scene_start();
    FV_DLL void scene_close();
    FV_DLL ObjectManager<GameObject>* gameObjectManager();
}
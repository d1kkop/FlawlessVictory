#pragma once
#include "../Core/ObjectManager.h"
#include "GameObject.h"
#include "SceneWorld.h"

namespace fv
{
    FV_DLL ObjectManager<GameObject>* gameObjectManager();
    FV_DLL ObjectManager<SceneWorld>* sceneWorldManager();
}
#pragma once
#include "ComponentManager.h"

namespace fn
{
    enum SceneType
    {
        Game,
        Temporal
    };

    class Scene
    {
        SceneComponent

    private:
        SceneType m_type = Game;
    };
}
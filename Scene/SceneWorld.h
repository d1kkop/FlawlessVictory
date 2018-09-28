#pragma once
#include "../Core/Common.h"

namespace fv
{
    class SceneComponent;

    class SceneWorld
    {
    public:

    private:
        void addSceneComponent(SceneComponent* component);
        void removeSceneComponent(SceneComponent* component);

    private:
        Set<SceneComponent*> m_Components;

        friend class SceneComponent;
    };
}
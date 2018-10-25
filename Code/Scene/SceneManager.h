#pragma once
#include "../Core/Common.h"
#include "SceneList.h"

namespace fv
{
    class SceneManager
    {
    public:
        FV_MO FV_DLL SceneList addScene(const String& name);
        FV_MO FV_DLL void removeScene(const String& name);
        FV_MO FV_DLL void serialize(const Path& filename, bool save);

    private:
        SceneList m_SceneLists[64]{};
    };


    FV_DLL class SceneManager* sceneManager();
    FV_DLL void deleteSceneManager();
}
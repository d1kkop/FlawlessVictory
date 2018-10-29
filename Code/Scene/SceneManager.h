#pragma once
#include "../Core/Common.h"
#include "SceneList.h"

namespace fv
{
    class SceneComponent;

    class SceneManager
    {
    public:
        FV_MO FV_DLL u64 addScene(const String& name); // returns sceneMask
        FV_MO FV_DLL void removeScene(const String& name);
        FV_MO FV_DLL void serialize(const String& name, const Path& filename, bool save);
        FV_MO FV_DLL u32 nextSceneComponentId() { return m_SceneComponentIdCounter++; }

        FV_MO FV_DLL u32 getIdFor( SceneComponent* comp );
        FV_MO FV_DLL void setIdFor( SceneComponent* comp, u32 id );
        FV_MO FV_DLL void setParentIdFor( SceneComponent* comp, u32 parentId );

    private:
        SceneList m_SceneLists[64]{};
        u32 m_SceneComponentIdCounter = 0;
        Map<SceneComponent*, u32> m_Comp2Id;
        Map<u32, SceneComponent*> m_Id2Comp;
    };


    FV_DLL class SceneManager* sceneManager();
    FV_DLL void deleteSceneManager();
}
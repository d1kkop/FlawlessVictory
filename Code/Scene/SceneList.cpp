#include "SceneList.h"
#include "SceneComponent.h"
#include "GameObject.h"
#include "../Core/Directories.h"
#include "../Core/LogManager.h"
#include "../Core/TextSerializer.h"
#include "../Core/ComponentManager.h"

namespace fv
{
    void SceneList::render(const Mat4& view, const Mat4& projection)
    {

    }

    void SceneList::serialize(const Path& filename, bool save)
    {
        FV_CHECK_MO();
        if ( save )
        {
            TextSerializer ts {};
            ts.serialize( "bit", m_Bit );
            ts.serialize( "name", m_Name );
            ts.pushArray();
            for ( SceneComponent& sc : Itr<SceneComponent>() )
            {
                ts.beginArrayElement();
                if ( sc.sceneBits() & (m_Bit) )
                {
                    GameObject* go = sc.gameObject();
                    if ( go )
                    {
                        go->serialize( ts );
                    }
                    else
                    {
                        LOGW("Found scene component without gameobject.");
                    }
                }
                ts.endArrayElement();
            }
            ts.popArray("gameObjects");
            ts.writeToFile( (Directories::scenes() / filename).string().c_str() );
        }
        else
        {
            TextSerializer ts( (Directories::scenes() / filename).string().c_str() );
        }
    }
}
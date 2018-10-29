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
            ts.pushArray("gameObjects");
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
            ts.popArray();
            ts.writeToFile( (Directories::scenes() / filename).string().c_str() );
        }
        else
        {
            TextSerializer ts( (Directories::scenes() / filename).string().c_str() );
            if (ts.hasSerializeErrors()) return;
            try
            {
                ts.pushArray("gameObjects");
                while ( ts.beginArrayElement() )
                {
                    GameObject* go = NewGameObject( false );
                    go->serialize( ts );
                    ts.endArrayElement();
                }
                ts.popArray();
            }
            catch (...)
            {
                LOGC("Failed to serialize %s.", filename.string().c_str());
            }
        }
    }
}
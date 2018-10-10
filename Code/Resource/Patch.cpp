#include "Patch.h"
#include "../Core/Thread.h"
#include "../Resource//Texture2D.h"

namespace fv
{
    void Patch::applyPatch()
    {
        switch ( patchType )
        {
        case PatchType::Texture2DLoad:
            applyTexture2DLoad();
            break;
        }
    }

    void Patch::applyTexture2DLoad()
    {
        FV_CHECK_MO();
        M<Texture2D> tex = std::static_pointer_cast<Texture2D>( resource );
        tex->applyPatch( width, height, imgFormat, graphic );
    }

}
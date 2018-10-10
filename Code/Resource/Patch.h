#pragma once
#include "../Core/Object.h"
#include "../Core/Common.h"
#include "../Render/GraphicResource.h"

namespace fv
{
    class Resource;

    enum class PatchType
    {
        Texture2DLoad
    };

    // Instead of subclassing this. Make one 'big' patch object to avoid massive memory fragmentation by different types.
    // Patches are reycled!
    class Patch: public Object
    {
    public:
        void applyPatch();

        PatchType patchType;
        GraphicResource* graphic {};
        M<Resource> resource;
        union
        {
            struct
            {
                u32 width, height, depth;
                ImageFormat imgFormat;
            }; /* Texture2 or 3D */
        };

    private:
        FV_MO void applyTexture2DLoad();
    };
}
#pragma once
#include "../Core/Common.h"

namespace fv
{
    struct ResourceToLoad;

    class TextureImporter
    {
    public:
        void load(const ResourceToLoad& rtl);
    };


    FV_DLL TextureImporter* textureImporter();
    FV_DLL void deleteTextureImporter();
}
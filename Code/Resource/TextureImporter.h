#pragma once
#include "../Core/Common.h"

namespace fv
{
    struct ResourceToLoad;

    class TextureImporter
    {
    public:
        bool reimport(const Path& path, u32& width, u32& height, enum class ImageFormat& format, Vector<byte>& data);
    };


    FV_DLL TextureImporter* textureImporter();
    FV_DLL void deleteTextureImporter();
}
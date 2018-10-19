#include "TextureImporter.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
#include "../Render/RenderManager.h"
#if FV_FREEIMAGE
    #include "../3rdParty/FreeImage/Dist/x64/FreeImage.h"
    #if _MSC_VER
    #pragma comment(lib, "../3rdParty/FreeImage/Dist/x64/FreeImage.lib")
    #endif
#endif

namespace fv
{
    bool TextureImporter::reimport(const Path& path, u32& width, u32& height, ImageFormat& imgFormat, Vector<byte>& data)
    {
        String sPath = path.string();
        const char* filename = sPath.c_str();

    #if FV_FREEIMAGE

        FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename, 0);
        if ( fif == FIF_UNKNOWN ) fif = FreeImage_GetFIFFromFilename(filename);
        if ( fif == FIF_UNKNOWN )
        {
            LOGW("Cannot obtain file type from texture %s.", filename);
            return false;
        }
        if ( !FreeImage_FIFSupportsReading(fif) )
        {
            LOGW("FreeImage does not support loading %s", filename);
            return false;
        }
        FIBITMAP* dib = FreeImage_Load(fif, filename);
        if ( !dib )
        {
            LOGW("Loading of %s failed.", filename);
            return false;
        }
        // Convert to easy format
        u32 bpp = FreeImage_GetBPP(dib);
        FIBITMAP* dibConverted;
        switch ( bpp )
        {
        case 8:
            dibConverted = FreeImage_ConvertTo8Bits(dib);
            imgFormat = ImageFormat::Single8;
            break;

        case 15:
            dibConverted = FreeImage_ConvertTo16Bits555(dib);
            imgFormat = ImageFormat::RGB555;
            break;

        case 24:
            dibConverted = FreeImage_ConvertTo24Bits(dib);
            imgFormat = ImageFormat::RGB8;
            break;

        default:
        case 32:
            dibConverted = FreeImage_ConvertTo32Bits(dib);
            imgFormat = ImageFormat::RGBA8;
            break;
        }
        FreeImage_Unload(dib);
        if ( !dibConverted )
        {
            LOGW("Could not convert %s to usuable format.", filename);
            return false;
        }

        byte* bits  = FreeImage_GetBits(dibConverted);
        width       = FreeImage_GetWidth(dibConverted);
        height      = FreeImage_GetHeight(dibConverted);
        // Sanity checks
        if ( (bits == nullptr) || (width == 0) || (height == 0) || (bpp % 8 != 0) )
        {
            LOGW("Texture %s has invalid data. Loading failed.", filename);
            FreeImage_Unload(dibConverted);
            return false;
        }
        u32 size = width*height*(bpp>>3);
        data.resize( size );
        memcpy( data.data(), bits, size );
        FreeImage_Unload(dibConverted);
    #endif

        return true;
    }


    TextureImporter* g_TextureImporter {};
    TextureImporter* textureImporter() { return CreateOnce(g_TextureImporter); }
    void deleteTextureImporter() { delete g_TextureImporter; g_TextureImporter=nullptr; }
}
#include "TextureImporter.h"
#include "../Core/Functions.h"
#include "../Core/Directories.h"
#include "../Core/LogManager.h"
#include "../Render/GraphicResource.h"
#include "../Render/RenderManager.h"
#include "../Resource/PatchManager.h"
#include "../Resource/ResourceManager.h"

namespace fv
{
    void TextureImporter::load(const ResourceToLoad& rtl)
    {
        assert( rtl.resource );

        String sPath = rtl.loadPath.string();
        const char* filename = sPath.c_str();

        u32 width, height;
        GraphicResource* graphic;

    #if FV_FREEIMAGE

        FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename, 0);
        if ( fif == FIF_UNKNOWN ) fif = FreeImage_GetFIFFromFilename(filename);
        if ( fif == FIF_UNKNOWN )
        {
            LOGW("Cannot obtain file type from texture %s.", filename);
            return;
        }
        if ( !FreeImage_FIFSupportsReading(fif) )
        {
            LOGW("FreeImage does not support loading %s", filename);
            return;
        }
        FIBITMAP* dib = FreeImage_Load(fif, filename);
        if ( !dib )
        {
            LOGW("Loading of %s failed.", filename);
            return;
        }
        // Convert to easy format
        u32 bpp = FreeImage_GetBPP(dib);
        FIBITMAP* dibConverted;
        ImageFormat imgFormat;
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
        case 16:
            dibConverted = FreeImage_ConvertTo16Bits565(dib);
            imgFormat = ImageFormat::RGB565;
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
            return;
        }

        byte* bits = FreeImage_GetBits(dibConverted);
        width  = FreeImage_GetWidth(dibConverted);
        height = FreeImage_GetHeight(dibConverted);
        // Sanity checks
        if ( (bits == nullptr) || (width == 0) || (height == 0) )
        {
            LOGW("Texture %s has invalid data. Loading failed.", filename);
            FreeImage_Unload(dibConverted);
            return;
        }

        graphic = nullptr; // renderManager()->createGraphic<Texture2D>(0 /*device idx*/);
        if ( !graphic )
        {
            LOGW("Cannot create graphic resource for texture 2D. Loading failed.", filename);
            FreeImage_Unload(dibConverted);
            return;
        }

        bool graphicUpdated = graphic->updateImage(width, height, bits, width*height*bpp, imgFormat);
        FreeImage_Unload(dibConverted);
        if ( !graphicUpdated )
        {
            renderManager()->freeGraphic(graphic);
            LOGW("Cannot update graphic resource for texture 2D. Loading failed.", filename);
            return;
        }

    #endif

        assert( width && height && graphic );
        Patch* patch      = patchManager()->createPatch(PatchType::Texture2DLoad);
        patch->width    = width;
        patch->height   = height;
        patch->graphic  = graphic;
        patch->resource = rtl.resource;

        patchManager()->submitPatch(patch);
    }


    TextureImporter* g_TextureImporter {};
    TextureImporter* textureImporter() { return CreateOnce(g_TextureImporter); }
    void deleteTextureImporter() { delete g_TextureImporter; g_TextureImporter=nullptr; }
}
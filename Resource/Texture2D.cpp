#include "Texture2D.h"
#include "../Core/JobManager.h"
#include "../Core/Directories.h"
#include "../Render/RenderManager.h"
#include "../Render/GraphicResource.h"

namespace fv
{
    FV_TYPE_IMPL(Texture2D)

    Texture2D::~Texture2D()
    {
        renderManager()->freeGraphic(m_Graphic);
    }

    void Texture2D::load(const Path& path)
    {
        bool graphicUpdated = false;

    #if FV_FREEIMAGE

        // TODO filename is incorrect from path ...
        const char* filename = path.string().c_str();
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
        m_Width    = FreeImage_GetWidth(dibConverted);
        m_Height   = FreeImage_GetHeight(dibConverted);
        // Sanity checks
        if ( (bits == nullptr) || (m_Width == 0) || (m_Width == 0) )
        {
            LOGW("Texture %s has invalid data. Loading failed.", filename);
            FreeImage_Unload(dibConverted);
            return;
        }
        m_Graphic = renderManager()->createGraphic<Texture2D>();
        graphicUpdated = m_Graphic->updateImage( m_Width, m_Height, bits, m_Width*m_Height*bpp, imgFormat );
        FreeImage_Unload(dibConverted);

    #endif

        if ( graphicUpdated )
        {
            m_LoadSuccesful = true;
        }
    }

    void Texture2D::onDoneOrCancelled(class Job* j)
    {
        j->waitAndFree();
        m_LoadDone = true;
    }

}
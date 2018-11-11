#include "Texture2D.h"
#include "PatchManager.h"
#include "ResourceManager.h"
#include "../Core/Thread.h"
#include "../Resource/TextureImporter.h"
#include "../Render/RenderManager.h"

namespace fv
{
    FV_TYPE_IMPL(Texture2D)

    Texture2D::~Texture2D()
    {
        renderManager()->deleteTexture2D( m_Graphic );
    }

    void Texture2D::applyPatch(u32 width, u32 height, ImageFormat format, const RTexture2D& graphic )
    {
        FV_CHECK_MO();
        renderManager()->deleteTexture2D( m_Graphic );
        m_Width  = width;
        m_Height = height;
        m_Format = format;
        m_Graphic = graphic;
    }

    void Texture2D::load(const ResourceToLoad& rtl)
    {
        u32 width, height;
        ImageFormat format;
        Vector<byte> data;
        if ( !textureImporter()->reimport( rtl.loadPath, width, height, format, data ) )
        {
            return;
        }

        RTexture2D graphic = renderManager()->createTexture2D(0, width, height, (const char*)data.data(), (u32)data.size(),
                                                              1, 1, 1, format );
        if ( !graphic )
        {
            LOGW("Cannot create graphic resource for texture 2D. Loading failed.", rtl.loadPath.string().c_str());
            return;
        }

        Patch* patch    = patchManager()->createPatch(PatchType::Texture2DData);
        patch->width    = width;
        patch->height   = height;
        patch->imgFormat = format;
        patch->texture2D = graphic;
        patch->resource  = rtl.resource;
        patch->submit();
    }

}
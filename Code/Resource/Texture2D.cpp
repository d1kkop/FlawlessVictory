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

    void Texture2D::applyPatch(u32 width, u32 height, ImageFormat format, u64 graphic)
    {
        FV_CHECK_MO();

        renderManager()->deleteTexture2D( m_Graphic );

        m_Width = width;
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

        u64 graphic = renderManager()->createTexture2D( width, height, (const char*)data.data(), (u32)data.size(), format );
        if ( graphic == -1 )
        {
            LOGW("Cannot create graphic resource for texture 2D. Loading failed.", rtl.loadPath.string().c_str());
            return;
        }

        Patch* patch    = patchManager()->createPatch(PatchType::Texture2DData);
        patch->width    = width;
        patch->height   = height;
        patch->graphic  = graphic;
        patch->resource = rtl.resource;
        patch->submit();
    }

}
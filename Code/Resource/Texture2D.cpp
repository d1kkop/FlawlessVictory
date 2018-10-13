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
        renderManager()->freeGraphic(m_Graphic, true);
    }

    void Texture2D::applyPatch(u32 width, u32 height, ImageFormat format, GraphicResource* resource)
    {
        FV_CHECK_MO();

        if ( m_Graphic ) 
            renderManager()->freeGraphic(m_Graphic, true);

        m_Width = width;
        m_Height = height;
        m_Format = format;
        m_Graphic = resource;
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

        GraphicResource* graphic = renderManager()->createGraphic(GraphicType::Texture2D, 0 /*device idx*/);
        if ( !graphic )
        {
            LOGW("Cannot create graphic resource for texture 2D. Loading failed.", rtl.loadPath.string().c_str());
            return;
        }

        bool graphicUpdated = graphic->updateImage( width, height, data.data(), (u32)data.size(), format );
        if ( !graphicUpdated )
        {
            renderManager()->freeGraphic(graphic);
            LOGW("Cannot update graphic resource for texture 2D. Loading failed.", rtl.loadPath.string().c_str());
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
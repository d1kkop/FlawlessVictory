#include "Texture2D.h"
#include "../Core/Thread.h"
#include "../Core/JobManager.h"
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
        textureImporter()->load( rtl );
    }

}
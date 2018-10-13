#pragma once
#include "Resource.h"
#include "../Render/GraphicResource.h"

namespace fv
{
    class Texture2D: public Resource
    {
        FV_TYPE(Texture2D)

    public:
        FV_DLL ~Texture2D() override;
        FV_DLL u32 width() const { return m_Width; }
        FV_DLL u32 height() const { return m_Height; }

        FV_MO void applyPatch( u32 width, u32 height, ImageFormat format, GraphicResource* resource );

    private:
        FV_DLL void load(const ResourceToLoad& rtl) override;

        u32 m_Width  = 0;
        u32 m_Height = 0;
        ImageFormat m_Format {};
        GraphicResource* m_Graphic{};
    };
}
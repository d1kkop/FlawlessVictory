#pragma once
#include "Resource.h"

namespace fv
{
    class Texture2D: public Resource
    {
        FV_TYPE(Texture2D)

    public:
        FV_DLL ~Texture2D() override;
        FV_DLL u32 width() const { return m_Width; }
        FV_DLL u32 height() const { return m_Height; }

        FV_MO void applyPatch( u32 width, u32 height, enum class ImageFormat format, u64 graphic );

    private:
        // From different thread
        FV_DLL void load(const ResourceToLoad& rtl) override;

        u32 m_Width  = 0;
        u32 m_Height = 0;
        u64 m_Graphic = -1;
        enum class ImageFormat m_Format {};
    };
}
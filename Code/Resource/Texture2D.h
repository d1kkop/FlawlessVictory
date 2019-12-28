#pragma once
#include "Resource.h"
#include "../Render/RenderManager.h"

namespace fv
{
    class Texture2D: public Resource
    {
        FV_TYPE(Texture2D)

    public:
        FV_DLL ~Texture2D() override;
        FV_DLL u32 width() const { return m_Width; }
        FV_DLL u32 height() const { return m_Height; }

        FV_MO void applyPatch( u32 width, u32 height, enum class ImageFormat format, const RTexture2D& graphic );

    private:
        // From different thread
        FV_DLL void load_RT(const ResourceToLoad& rtl) override;

        u32 m_Width  = 0;
        u32 m_Height = 0;
        RTexture2D m_Graphic{};
        enum class ImageFormat m_Format {};
    };

    using Tex2D = M<Texture2D>;
}
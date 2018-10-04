#pragma once
#include "Resource.h"

namespace fv
{
    class Texture2D: public Resource
    {
        FV_RESOURCE_TYPE(Texture2D, "")

    public:
        void load(const Path& path) override;
        void onDoneOrCancelled(class Job* j) override;

        u32 m_Width  = 0;
        u32 m_Height = 0;
        u32 m_Channels = 0;
        class GraphicResource* m_Graphic{};
    };
}
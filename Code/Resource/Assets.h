#pragma once
#include "../Core/Common.h"

namespace fv
{
    class Assets
    {
    public:
        // Extensions
        static const Path& importExtension();
        static const Path& metaExtension();
        static const Path& meshBinExtension();
        static const Path& tex2DBinExtension();
        static const Path& shaderBinExtension();
    };
}
#pragma once
#include "../Core/Common.h"

namespace fv
{
    class TextSerializer;

    class SceneList
    {
    public:
        void render(const Mat4& view, const Mat4& projection);
        FV_MO FV_DLL void serialize(const Path& filename, bool save);

        bool valid() const { return m_Valid; }
        u64 bit() const { return m_Bit; }

    private:
        bool m_Valid = false;
        u64 m_Bit = 1;
        String m_Name;

        friend class SceneManager;
    };
}
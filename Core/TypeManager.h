#pragma once
#include "Common.h"

namespace fv
{
    class TypeManager
    {
    public:
        FV_DLL u32 nameToType(const char* name);

    private:
        u32 m_TypeCounter{};
        Map<String, u32> m_NameToType;
    };
}
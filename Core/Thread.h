#pragma once
#include "Common.h"
#include <thread>

namespace fv
{
    class Thread: public std::thread
    {
    public:
        Thread() = default;
        Thread(const String& name);
        void setName(const String& name);
    };
}

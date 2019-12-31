#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class HelperVK
    {
    public:
        template <typename From, typename To> 
        static void toVkList(const List<From>& from, List<To>& to)
        {
            for ( auto& f : from ) 
                to.emplace_back( f->vk() );
        }
    };
}
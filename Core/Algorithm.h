#pragma once
#include <algorithm>

namespace fv
{
    template <class Container, class Pred>
    void sort(Container& c, Pred pred)
    {
        std::sort(c.begin(), c.end(), pred);
    }

    template <class Container, class Pred>
    auto find_if(Container& c, Pred pred)
    {
        return std::find_if( c.begin(), c.end(), pred );
    }

    template <class Container, class Pred>
    bool remove_if(Container& c, Pred pred)
    {
        auto it = find_if(c, pred);
        if ( it != c.end() )
        {
            c.erase( it );
            return true;
        }
        return false;
    }
}
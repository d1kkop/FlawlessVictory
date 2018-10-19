#pragma once
#include <algorithm>

namespace fv
{
    template <class Container, class Pred>
    void Sort(Container& c, Pred pred)
    {
        std::sort(c.begin(), c.end(), pred);
    }

    template <class Container, class Pred>
    auto Find_if(Container& c, Pred pred)
    {
        return std::find_if( c.begin(), c.end(), pred );
    }

    template <class Container, class Pred>
    bool Remove_if(Container& c, Pred pred)
    {
        auto it = Find_if(c, pred);
        if ( it != c.end() )
        {
            c.erase( it );
            return true;
        }
        return false;
    }

    template <class Container, class Value>
    bool RemoveMemCmp(Container& c, Value& v)
    {
        auto it = Find_if(c, [&](auto& e)
        {
            return memcmp(&e, &v, sizeof(Value))==0;
        });
        if ( it != c.end() )
        {
            c.erase(it);
            return true;
        }
        return false;
    }

    template <class Container, class Value>
    bool Contains(const Container& c, const Value& v)
    {
        return Find_if ( c, [&](auto& cl) { return v==cl; } ) != c.end();
    }

    template <class Container, class Value>
    void Remove(Container& c, const Value& v)
    {
        Remove_if( c, [&](auto& cl) { return v==cl; });
    }
}
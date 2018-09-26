#pragma once
#include <vector>
#include <map>
#include <set>

namespace fv
{
    using byte = unsigned char;
    using i16  = short;
    using i32  = long;
    using i64  = long long;
    using u16  = unsigned short;
    using u32  = unsigned long;
    using u64  = unsigned long long;

    template <class T> using Array = std::vector<T>;
    template <class T> using Set = std::set<T>;
    template <class Key, class Value> using Map = std::map<Key, Value>;
    using String = std::string;

    template <class T, class F>
    inline T rc (F&& f) { return reinterpret_cast<T>(f); }
    template <class T, class F>
    inline T sc (F&& f) { return static_cast<T>(f); }
    template <class T, class F>
    inline T scc (F&& f) { return static_cast<T>(const_cast<T>(f)); }
    template <class T>
    void deleteAndNull(T*& ptr) { delete ptr; ptr = nullptr; }
}

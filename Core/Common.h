#pragma once
#include "Platform.h"
#include <vector>
#include <map>
#include <set>
#include <string>
#include <memory>
#include <mutex>

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
    template <class T> using Set   = std::set<T>;
    template <class Key, class Value> using Map = std::map<Key, Value>;
    using String = std::string;

    using Mutex         = std::mutex;
    using RMutex        = std::recursive_mutex;
    using scoped_lock   = std::lock_guard<Mutex>;
    using rscoped_lock  = std::lock_guard<RMutex>;

    template <class T, class F>
    inline T rc (F&& f) { return reinterpret_cast<T>(f); }
    template <class T, class F>
    inline T sc (F&& f) { return static_cast<T>(f); }
    template <class T, class F>
    inline T scc (F&& f) { return static_cast<T>(const_cast<T>(f)); }

    // Managed ptr
    template <class T> using M = std::shared_ptr<T>;

    // Only a single refcounted ptr
    template <class T> using U = std::unique_ptr<T>;

    // Can become null if owning shared ptr loses its reference
    template <class T> using W = std::weak_ptr<T>;


    // ------------- Functions -----------------------------------------

    // Returns local time as string.
    String localTime();
    String format(const char* msg, ...);
    template <class T> 
    void deleteAndNull(T*& ptr) { delete ptr; ptr = nullptr; }
}

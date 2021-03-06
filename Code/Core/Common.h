#pragma once
#include "Platform.h"
#include "PCH.h"

namespace fv
{
    using byte = unsigned char;
    using i16  = short;
    using i32  = long;
    using i64  = long long;
    using u16  = unsigned short;
    using u32  = unsigned long;
    using u64  = unsigned long long;

    template <class T, class S> using Pair = std::pair<T, S>;
    template <class T> using Vector  = std::vector<T>;
	template <class T> using List    = Vector<T>;
    template <class T> using Queue   = std::queue<T>;
    template <class T> using Stack   = std::stack<T>;
    template <class T> using Deck    = std::deque<T>;
    template <class T> using Set     = std::set<T>;
    template <class T> using HashSet = std::unordered_set<T>;
    template <class T> using Function = std::function<T>;
    template <class T> using Optional = std::optional<T>;
    template <class Key, class Value> using Map = std::map<Key, Value>;
    template <class Key, class Value> using HashMap = std::unordered_map<Key, Value>;

    using String        = std::string;
    using Path          = std::filesystem::path;
    using CondVar       = std::condition_variable;

    template <class T>
    using Atomic        = std::atomic<T>;
    using Thread        = std::thread;
    using Mutex         = std::mutex;
    using RMutex        = std::recursive_mutex;
    using Lock          = std::scoped_lock<Mutex>;
    using RLock         = std::scoped_lock<RMutex>;

    // Managed ptr
    template <class T> using M = std::shared_ptr<T>;

    // Only a single refcounted ptr
    template <class T> using U = std::unique_ptr<T>;

    // Can become null if owning shared ptr loses its reference
    template <class T> using W = std::weak_ptr<T>;

    template <class T, class F>
    inline T rc (F&& f) { return reinterpret_cast<T>(f); }
    template <class T, class F>
    inline T sc (F&& f) { return static_cast<T>(f); }
    template <class T, class F>
    inline T scc (const F&& f) { return static_cast<T>(const_cast<T>(f)); }
	template <class T, class F>
    inline M<T> spc( F&& f ) { return std::static_pointer_cast<T>(f); }
    template <class T, class F>
    inline M<T> dpc( F&& f ) { return std::dynamic_pointer_cast<T>(f); }
    template <class T, class F>
    inline M<T> rpc( F&& f ) { return std::reinterpret_pointer_cast<T>(f); }
}

#include "PCH.h"
#include "TypeManager.h"
#include "Functions.h"
#include "LogManager.h"
#include <cassert>

namespace fv
{
    u32 TypeManager::registerType(const char* name, u32 size, CreateFunc cfunc, ResetFunc rfunc, u32 flags)
    {
        assert( name && size && cfunc && rfunc );
        auto nIt = m_NameToType.find(name);
        if ( nIt == m_NameToType.end() )
        {
            String sName = name;

            // While Name does not resolve to an existing ID, keep adding dummy characters until unique ID is found.
            u32 hash = Hash32(sName.c_str(), (u32)sName.size());
            while ( m_HashToType.count(hash) != 0 || hash == -1/*Invalid hash value*/ )
            {
                sName += "a"; // Add dummy character so that it will resolve to different ID.
                hash = Hash32(sName.c_str(), (u32)sName.size());
            }

            // Use original name to obtain chosen hash ID
            TypeInfo ti;
            {
                ti.hash=hash;
                ti.size=size;
                ti.createFunc=cfunc;
                ti.resetFunc=rfunc;
                ti.name=nullptr; // Name stored in m_NameToType, ptr assigned to that value below.
                ti.flags = flags;
            }
            m_NameToType[name] = ti;
            m_HashToType[hash] = &m_NameToType[name];
            auto tIt = m_NameToType.find(name);
            assert( tIt != m_NameToType.end() );
            tIt->second.name = &tIt->first;
        }
        else
        {
            LOGW("Type %s already registered.", name);
        }
        return typeInfo(name)->hash;
    }

    FV_TS const TypeInfo* TypeManager::typeInfo(const char* name)
    {
        auto nIt = m_NameToType.find(name);
        if ( nIt != m_NameToType.end() )
        {
            return &nIt->second;
        }
        LOGW( "Cannot find type: %s.", name );
        return nullptr;
    }

    FV_TS const TypeInfo* TypeManager::typeInfo(u32 hash)
    {
        auto nIt = m_HashToType.find(hash);
        if ( nIt != m_HashToType.end() )
        {
            return nIt->second;
        }
        LOGW("Cannot find type hash: %d.", hash);
        return nullptr;
    }

    void TypeManager::setType(u32 type, Type& t)
    {
        t.m_Type = type;
    }

    TypeManager* g_TypeManager {};
    TypeManager* typeManager() { return CreateOnce(g_TypeManager); }
    void deleteTypeManager() { delete g_TypeManager; g_TypeManager=nullptr; }
}
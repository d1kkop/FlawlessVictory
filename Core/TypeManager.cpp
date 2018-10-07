#include "PCH.h"
#include "TypeManager.h"
#include "Functions.h"
#include "LogManager.h"
#include <cassert>

namespace fv
{
    u32 TypeManager::registerType(const char* name, u32 size, CreateFunc cfunc, ResetFunc rfunc, i32 updatePriority, const char* extension)
    {
        assert( name && size && cfunc && rfunc );
        auto nIt = m_NameToType.find(name);
        if ( nIt == m_NameToType.end() )
        {
            String sName = name;

            // While Name does not resolve to an existing ID, keep adding dummy characters until unique ID is found.
            u32 hash = Hash32(sName.c_str(), (u32)sName.size());
            while ( m_HashToType.count(hash) != 0 )
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
                ti.updatePriority=updatePriority;
                StringCopy(ti.extension, FV_MAX_EXTENSION, extension);
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

    const TypeInfo* TypeManager::typeInfo(const char* name)
    {
        auto nIt = m_NameToType.find(name);
        if ( nIt != m_NameToType.end() )
        {
            return &nIt->second;
        }
        LOGC( "Cannot find type: %s.", name );
        return nullptr;
    }

    const TypeInfo* TypeManager::typeInfo(u32 hash)
    {
        auto nIt = m_HashToType.find(hash);
        if ( nIt != m_HashToType.end() )
        {
            return nIt->second;
        }
        LOGC("Cannot find type hash: %d.", hash);
        return nullptr;
    }

    Type* TypeManager::createTypes(u32 type, u32 num)
    {
        const TypeInfo* ti = typeInfo(type);
        if (!ti) return nullptr;
        assert(type == ti->hash);
        return createTypes(*ti, num);
    }

    Type* TypeManager::createTypes(const TypeInfo& ti, u32 num)
    {
        Type* types = ti.createFunc(num);
        for ( u32 i=0; i<num; ++i )
        {
            ((Type*)((char*)types + i*ti.size))->m_Type = ti.hash;
        }
        return types;
    }

    void TypeManager::setType(u32 type, Type& t)
    {
        t.m_Type = type;
    }

    TypeManager* g_TypeManager {};
    TypeManager* typeManager() { return CreateOnce(g_TypeManager); }
    void deleteTypeManager() { delete g_TypeManager; g_TypeManager=nullptr; }
}
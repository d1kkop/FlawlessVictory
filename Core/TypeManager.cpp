#include "TypeManager.h"
#include "Functions.h"
#include "CoreMain.h"

namespace fv
{
    TypeInfo InvalidType = { (u32)-1, nullptr, nullptr, nullptr };

    u32 TypeManager::registerType(const char* name, CreateFunc cfunc, ResetFunc rfunc)
    {
        auto nIt = m_NameToType.find(name);
        if ( nIt == m_NameToType.end() )
        {
            String sName = name;

            // While Name does not resolve to an existing ID, keep adding dummy characters until unique ID is found.
            u32 hash = hash32(sName.c_str(), (u32)sName.size());
            while ( m_HashToType.count(hash) != 0 )
            {
                sName += "a"; // Add dummy character so that it will resolve to different ID.
                hash = hash32(sName.c_str(), (u32)sName.size());
            }

            // Use original name to obtain chosen hash ID
            TypeInfo ti = { hash, cfunc, rfunc, nullptr };
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
        return typeInfo(name).hash;
    }

    TypeInfo& TypeManager::typeInfo(const char* name)
    {
        auto nIt = m_NameToType.find(name);
        if ( nIt != m_NameToType.end() )
        {
            return nIt->second;
        }
        LOGC( "Cannot find type: %s.", name );
        return InvalidType;
    }

    FV_DLL TypeInfo& TypeManager::typeInfo(u32 hash)
    {
        auto nIt = m_HashToType.find(hash);
        if ( nIt != m_HashToType.end() )
        {
            return *nIt->second;
        }
        LOGC("Cannot find type hash: %d.", hash);
        return InvalidType;
    }

}
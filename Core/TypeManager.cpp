#include "TypeManager.h"
#include "Math.h"

namespace fv
{
    u32 TypeManager::nameToType(const char* name)
    {
        auto nIt = m_NameToType.find( name );
        if ( nIt == m_NameToType.end() )
        {
            m_NameToType[name] = Hash32( name, (u32) strlen(name) );
        }
        return m_NameToType[name];
    }

}
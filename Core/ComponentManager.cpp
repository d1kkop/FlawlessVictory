#include "ComponentManager.h"

namespace fv
{
    ComponentManager::~ComponentManager()
    {
        for ( auto& kvp : m_ActiveComponents )
            for ( auto* c : kvp.second ) delete c;
        for ( auto& kvp : m_FreeComponents )
            for ( auto* c : kvp.second ) delete c;
    }
}
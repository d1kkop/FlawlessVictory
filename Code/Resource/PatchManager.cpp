#include "PatchManager.h"
#include "Patch.h"
#include "../Core/Functions.h"
#include "../Core/Thread.h"

namespace fv
{
    PatchManager::PatchManager():
        m_PatchPool(16, true)
    {
    }

    Patch* PatchManager::createPatch(PatchType type)
    {
        Patch* p = m_PatchPool.newObject();
        p->patchType = type;
        return p;
    }

    void PatchManager::freePatch(Patch* p)
    {
        assert(p);
        if ( !p ) return;
        m_PatchPool.freeObject(p);
    }

    void PatchManager::submitPatch(Patch* p)
    {
        assert( p );
        scoped_lock lk(m_PatchSubmitMutex);
        m_PatchSubmitList.emplace_back( p );
    }

    void PatchManager::applyPatches()
    {
        FV_CHECK_MO();
        Vector<Patch*> submitListCpy;
        {
            scoped_lock lk(m_PatchSubmitMutex);
            submitListCpy = m_PatchSubmitList;
            m_PatchSubmitList.clear();
        }
        for ( auto* p : submitListCpy )
        {
            p->applyPatch();
            freePatch( p );
        }
    }


    PatchManager* g_PatchManager {};
    PatchManager* patchManager() { return CreateOnce(g_PatchManager); }
    void deletePatchManager() { delete g_PatchManager; g_PatchManager=nullptr; }

}
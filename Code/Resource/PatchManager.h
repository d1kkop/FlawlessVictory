#pragma once
#include "../Core/SparseArray.h"
#include "Patch.h"

namespace fv
{
    class Patch;

    class PatchManager
    {
    public:
        PatchManager();
        FV_TS Patch* createPatch(PatchType patchType);
        FV_TS void freePatch(Patch* p);

        FV_TS void submitPatch( Patch* p );
        FV_MO FV_DLL void applyPatches();

    private:
        Mutex m_PatchSubmitMutex;
        Vector<Patch*> m_PatchSubmitList;
        SparseArray<Patch> m_PatchPool;
    };


    FV_DLL PatchManager* patchManager();
    FV_DLL void deletePatchManager();
}
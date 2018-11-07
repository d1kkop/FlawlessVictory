#include "PCH.h"
#include "TransformManager.h"
#include "Functions.h"
#include "Thread.h"

namespace fv
{
    Mat4* TransformManager::newLocalToWorldMatrix()
    {
        FV_CHECK_MO();
        return m_LocalToWorldMatrices.newObject();
    }

    void TransformManager::freeLocalToWorldMatrix(Mat4* mat)
    {
        FV_CHECK_MO();
        m_LocalToWorldMatrices.freeObject( mat );
    }

    Mat4* TransformManager::newWorldToLocalMatrix()
    {
        FV_CHECK_MO();
        return m_WorldToLocalMatrices.newObject();
    }

    void TransformManager::freeWorldToLocalMatrix(Mat4* mat)
    {
        FV_CHECK_MO();
        m_WorldToLocalMatrices.freeObject( mat );
    }


    TransformManager* g_TransformManager {};
    TransformManager* transformManager() { return CreateOnce(g_TransformManager); }
    void deleteTransformManager() { delete g_TransformManager; g_TransformManager=nullptr; }

}
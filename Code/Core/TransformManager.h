#pragma once
#include "SparseArray2.h"

namespace fv
{
    class TransformManager
    {
    public:
        FV_MO FV_DLL Mat4* newLocalToWorldMatrix();
        FV_MO FV_DLL void freeLocalToWorldMatrix(Mat4* mat);
        FV_MO FV_DLL Mat4* newWorldToLocalMatrix();
        FV_MO FV_DLL void freeWorldToLocalMatrix(Mat4* mat);

    private:
        SparseArray2<Mat4> m_LocalToWorldMatrices;
        SparseArray2<Mat4> m_WorldToLocalMatrices;
    };

    FV_MO FV_DLL TransformManager* transformManager();
    FV_MO FV_DLL void deleteTransformManager();
}
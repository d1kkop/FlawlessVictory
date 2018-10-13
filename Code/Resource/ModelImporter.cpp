#include "ModelImporter.h"
#include "ResourceManager.h"
#include "../Core/LogManager.h"
#include "../Core/Functions.h"
using namespace std;
#if FV_ASSIMP
using namespace Assimp;
#endif

namespace fv
{
    Path ModelImporter::replaceWithBinaryExtension(const Path& path)
    {
        Path p;
        p.replace_extension( ".mesh" );
        return p;
    }

    bool ModelImporter::writeBinary(const Path& path, const Vector<Submesh>& submeshes)
    {
        return true;
    }

    bool ModelImporter::loadBinary(const Path& path, Vector<Submesh>& submeshes)
    {
        return true;
    }

    bool ModelImporter::reimport(const Path& path, Vector<Submesh>& submeshes)
    {

     #if FV_ASSIMP
        Importer importer;
        const aiScene* scene = importer.ReadFile(path.string().c_str(), aiProcessPreset_TargetRealtime_Fast); // aiProcessPreset_TargetRealtime_Fast aiProcessPreset_TargetRealtime_MaxQuality

        if ( !scene )
        {
            LOGW("Failed to import %s.", path.string().c_str());
            return true;
        }

        submeshes.resize( scene->mNumMeshes );
        for ( u32 i = 0; i < scene->mNumMeshes; i++ )
        {
            const aiMesh* aiMesh = scene->mMeshes[i];
            auto& sm = submeshes[i];
            // determine bounding box min/max
            sm.bMin = { FLT_MAX, FLT_MAX, FLT_MAX };
            sm.bMax = { FLT_MIN, FLT_MIN, FLT_MIN };
            for ( u32 j=0; j<aiMesh->mNumVertices; ++j )
            {
                sm.bMin.min( *(Vec3*)&aiMesh->mVertices[j] );
                sm.bMax.max( *(Vec3*)&aiMesh->mVertices[j] );
            }
            sm.vertices.resize( aiMesh->mNumVertices );
            memcpy( sm.vertices.data(), aiMesh->mVertices, aiMesh->mNumVertices*sizeof(Vec3) );
            if ( aiMesh->HasNormals() ) 
            {
                sm.normals.resize( aiMesh->mNumVertices );
                memcpy( sm.normals.data(), aiMesh->mNormals, aiMesh->mNumVertices*sizeof(Vec3) );
            }
            if ( aiMesh->HasTangentsAndBitangents() )
            {
                sm.tangents.resize( aiMesh->mNumVertices );
                sm.bitangents.resize( aiMesh->mNumVertices );
                memcpy( sm.tangents.data(), aiMesh->mTangents, aiMesh->mNumVertices*sizeof(Vec3) );
                memcpy( sm.bitangents.data(), aiMesh->mBitangents, aiMesh->mNumVertices*sizeof(Vec3) );
            }
            if ( aiMesh->GetNumUVChannels() > 0 ) // normal uvs
            {
                sm.uvs.resize( aiMesh->mNumVertices );
                for ( u32 k=0; k<aiMesh->mNumVertices; ++k )
                {
                    sm.uvs[k].x = aiMesh->mTextureCoords[0][k].x;
                    sm.uvs[k].y = aiMesh->mTextureCoords[0][k].y;
                }
            }
            if ( aiMesh->GetNumUVChannels() > 1 ) // light uvs
            {
                sm.lightUVs.resize(aiMesh->mNumVertices);
                for ( u32 k=0; k<aiMesh->mNumVertices; ++k )
                {
                    sm.lightUVs[k].x = aiMesh->mTextureCoords[1][k].x;
                    sm.lightUVs[k].y = aiMesh->mTextureCoords[1][k].y;
                }
            }
            Vector<Vec4>* extras [] = { &sm.extra1, &sm.extra2, &sm.extra3, &sm.extra4 };
            for ( u32 j=0; j<4; j++ )
            {
                if ( aiMesh->GetNumColorChannels() > j )
                {
                    extras[j]->resize( aiMesh->mNumVertices );
                    memcpy( extras[j]->data(), aiMesh->mColors[j], aiMesh->mNumVertices*sizeof(Vec4) );
                }
            }
            sm.indices.resize( aiMesh->mNumFaces * 3 );
            for ( u32 j=0; j<aiMesh->mNumFaces; j++ )
            {
                sm.indices[j*3+0] = aiMesh->mFaces[j].mIndices[0];
                sm.indices[j*3+1] = aiMesh->mFaces[j].mIndices[1];
                sm.indices[j*3+2] = aiMesh->mFaces[j].mIndices[2];
            }
        }
    #endif

        Path output = replaceWithBinaryExtension( path );
        writeBinary( output, submeshes );

        return true;
    }

    ModelImporter* g_ModelImporter {};
    ModelImporter* modelImporter() { return CreateOnce(g_ModelImporter); }
    void deleteModelImporter() { delete g_ModelImporter; g_ModelImporter=nullptr; }
}
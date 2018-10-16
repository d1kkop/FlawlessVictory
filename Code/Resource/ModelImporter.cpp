#include "ModelImporter.h"
#include "ResourceManager.h"
#include "Assets.h"
#include "Material.h"
#include "Texture2D.h"
#include "../Core/LogManager.h"
#include "../Core/Functions.h"
#include "../Core/BinarySerialize.h"
#include "../Core/Directories.h"
#include "../Core/TextSerializer.h"
#include "../Render/RenderManager.h"
using namespace std;
#if FV_ASSIMP
using namespace Assimp;
#endif

namespace fv
{

    // ---------- MeshImportSettings ---------------------------------------------------------------------------------------------

    bool MeshImportSettings::read(const Path& path)
    {   
        Path importPath = Directories::intermediateMeshes() / path.filename();
        importPath.replace_extension(Assets::importExtension());
        TextSerializer importSerializer( importPath.string().c_str() );
        if ( !importSerializer.hasSerializeErrors() )
        {
            serialize(importSerializer);
        }
        return !importSerializer.hasSerializeErrors();
    }

    bool MeshImportSettings::write(const Path& path)
    {
        Path importPath = Directories::intermediateMeshes() / path.filename();
        importPath.replace_extension(Assets::importExtension());
        TextSerializer importSerializer;
        serialize(importSerializer);
        importSerializer.writeToFile(importPath.string().c_str());
        return !importSerializer.hasSerializeErrors();
    }

    void MeshImportSettings::serialize(TextSerializer& serializer)
    {
        serializer.serialize( "keepInRam", keepInRam );
    }

    // ---------- ModelImporter --------------------------------------------------------------------------------------------------

    bool ModelImporter::writeBinary(const Path& path, const Vector<Submesh>& submeshes)
    {
        ofstream file(path.string().c_str(), ios::binary);
        if ( file.is_open() )
        {
            try
            {
                u32 numSubmeshes = (u32)submeshes.size();
                BinWrite( file, numSubmeshes );
                for ( u32 i=0; i<numSubmeshes; ++i )
                {
                    BinWrite( file, submeshes[i].bMin );
                    BinWrite( file, submeshes[i].bMax );
                    BinWriteVector( file, submeshes[i].indices );
                    BinWriteVector( file, submeshes[i].vertices );
                    BinWriteVector( file, submeshes[i].normals );
                    BinWriteVector( file, submeshes[i].tangents );
                    BinWriteVector( file, submeshes[i].bitangents );
                    BinWriteVector( file, submeshes[i].uvs );
                    BinWriteVector( file, submeshes[i].lightUVs );
                    BinWriteVector( file, submeshes[i].extra1 );
                    BinWriteVector( file, submeshes[i].extra2 );
                    BinWriteVector( file, submeshes[i].extra3 );
                    BinWriteVector( file, submeshes[i].extra4 );
                    BinWriteVector( file, submeshes[i].boneIndices );
                    BinWriteVector( file, submeshes[i].weights );
                }
                file.close();
                return true;
            }
            catch (...)
            {
                if ( file.is_open() ) file.close();
            }
        }
        return false;
    }

    bool ModelImporter::loadBinary(const Path& path, Vector<Submesh>& submeshes)
    {
        ifstream file(path.string().c_str(), ios::binary);
        if ( file.is_open() )
        {
            try
            {
                u32 numSubmeshes = 0;
                BinRead(file, numSubmeshes);
                if ( numSubmeshes > 10000 )
                {
                    LOGW("More than 10000 submeshes detected in model. Strange high number. Load failed for %s.", path.string().c_str());
                    file.close();
                    return false;
                }
                submeshes.resize( numSubmeshes );
                for ( u32 i=0; i<numSubmeshes; ++i )
                {
                    BinRead(file, submeshes[i].bMin);
                    BinRead(file, submeshes[i].bMax);
                    BinReadVector(file, submeshes[i].indices);
                    BinReadVector(file, submeshes[i].vertices);
                    BinReadVector(file, submeshes[i].normals);
                    BinReadVector(file, submeshes[i].tangents);
                    BinReadVector(file, submeshes[i].bitangents);
                    BinReadVector(file, submeshes[i].uvs);
                    BinReadVector(file, submeshes[i].lightUVs);
                    BinReadVector(file, submeshes[i].extra1);
                    BinReadVector(file, submeshes[i].extra2);
                    BinReadVector(file, submeshes[i].extra3);
                    BinReadVector(file, submeshes[i].extra4);
                    BinReadVector(file, submeshes[i].boneIndices);
                    BinReadVector(file, submeshes[i].weights);
                }
                file.close();
                return true;
            }
            catch (...)
            {
                if ( file.is_open() ) file.close();
            }
        }
        return false;
    }

    bool ModelImporter::reimport(const Path& path, const MeshImportSettings& settings, Vector<Submesh>& submeshes, Vector<M<Material>>& materials)
    {

     #if FV_ASSIMP
        Importer importer;
        const aiScene* scene = importer.ReadFile(path.string().c_str(), aiProcessPreset_TargetRealtime_Fast); // aiProcessPreset_TargetRealtime_Fast aiProcessPreset_TargetRealtime_MaxQuality
        if ( !scene )
        {
            LOGW("Failed to import %s from assimp.", path.string().c_str());
            return true;
        }
        loadSubmeshes(submeshes, scene);
        loadMaterials(path.filename().replace_extension("").string(), materials, scene);
    #endif

        // Write binary conversion
        Path output = Directories::intermediateMeshes() / path.filename();
        output.replace_extension( Assets::meshBinExtension() );
        writeBinary( output, submeshes );

        return true;
    }

#if FV_ASSIMP
    void ModelImporter::loadSubmeshes(Vector<Submesh> &submeshes, const aiScene* scene)
    {
        if ( scene->mNumMeshes==0 ) return;
        submeshes.resize(scene->mNumMeshes);
        for ( u32 i = 0; i < scene->mNumMeshes; i++ )
        {
            const aiMesh* aiMesh = scene->mMeshes[i];
            auto& sm = submeshes[i];
            // determine bounding box min/max
            sm.bMin = { FLT_MAX, FLT_MAX, FLT_MAX };
            sm.bMax = { FLT_MIN, FLT_MIN, FLT_MIN };
            for ( u32 j=0; j<aiMesh->mNumVertices; ++j )
            {
                sm.bMin.min(*(Vec3*)&aiMesh->mVertices[j]);
                sm.bMax.max(*(Vec3*)&aiMesh->mVertices[j]);
            }
            // vertices
            sm.vertices.resize(aiMesh->mNumVertices);
            memcpy(sm.vertices.data(), aiMesh->mVertices, aiMesh->mNumVertices*sizeof(Vec3));
            // normals
            if ( aiMesh->HasNormals() )
            {
                sm.normals.resize(aiMesh->mNumVertices);
                memcpy(sm.normals.data(), aiMesh->mNormals, aiMesh->mNumVertices*sizeof(Vec3));
            }
            // tangents
            if ( aiMesh->HasTangentsAndBitangents() )
            {
                sm.tangents.resize(aiMesh->mNumVertices);
                sm.bitangents.resize(aiMesh->mNumVertices);
                memcpy(sm.tangents.data(), aiMesh->mTangents, aiMesh->mNumVertices*sizeof(Vec3));
                memcpy(sm.bitangents.data(), aiMesh->mBitangents, aiMesh->mNumVertices*sizeof(Vec3));
            }
            // uvs
            Vector<Vec2>* uvs[] = { &sm.uvs, &sm.lightUVs };
            for ( u32 j=0; j<Min(aiMesh->GetNumUVChannels(), 2U); j++ )
            {
                uvs[j]->resize(aiMesh->mNumVertices);
                for ( u32 k=0; k<aiMesh->mNumVertices; ++k )
                {
                    sm.uvs[k].x = aiMesh->mTextureCoords[j][k].x;
                    sm.uvs[k].y = aiMesh->mTextureCoords[j][k].y;
                }
            }
            // extras
            Vector<Vec4>* extras[] = { &sm.extra1, &sm.extra2, &sm.extra3, &sm.extra4 };
            for ( u32 j=0; j<Min(4u, aiMesh->GetNumColorChannels()); j++ )
            {
                extras[j]->resize(aiMesh->mNumVertices);
                memcpy(extras[j]->data(), aiMesh->mColors[j], aiMesh->mNumVertices*sizeof(Vec4));
            }
            // indices
            sm.indices.resize(aiMesh->mNumFaces * 3);
            for ( u32 j=0; j<aiMesh->mNumFaces; j++ )
            {
                sm.indices[j*3+0] = aiMesh->mFaces[j].mIndices[0];
                sm.indices[j*3+1] = aiMesh->mFaces[j].mIndices[1];
                sm.indices[j*3+2] = aiMesh->mFaces[j].mIndices[2];
            }
        }
    }

    Binding ModelImporter::aiTextureTypeToBinding(aiTextureType type)
    {
        switch ( type )
        {
        case aiTextureType_DIFFUSE:
            return Binding::Diffuse;
        case aiTextureType_SPECULAR:
            return Binding::Specular;
        case aiTextureType_AMBIENT:
            return Binding::Ambient;
        case aiTextureType_EMISSIVE:
            return Binding::Emissive;
        case aiTextureType_NORMALS:
            return Binding::Normal;
        }
        LOGW("Cannot map Assimp texture type to correct binding. Defaultin to 'diffuse'.");
        return Binding::Diffuse;
    }

    void ModelImporter::aiAddSamplerFromModel( const aiMaterial* aiMat, Material* mat, aiTextureType texType )
    {
        assert( aiMat && mat );
        aiString pathToTexture;
        if ( aiMat->GetTexture( texType, 0, &pathToTexture) == aiReturn_SUCCESS &&
             (pathToTexture.length==0 || pathToTexture.data[0] != '*') /* Do not import embedded textures for now */ )
        {
            Sampler2D s {};
            s.filter   = SamplerFilter::An16x;
            s.texture  = resourceManager()->load<Texture2D>(pathToTexture.C_Str());
            s.location = (u32) aiTextureTypeToBinding( texType );
            mat->m_Samplers.emplace_back(s);
        }
    }

    void ModelImporter::loadMaterials(const String& baseName, Vector<M<Material>>& materials, const aiScene* scene)
    {
        if ( scene->mNumMaterials==0 ) return;
        materials.resize( scene->mNumMaterials );
        for ( u32 i = 0; i < scene->mNumMaterials; i++ )
        {
            const aiMaterial* aiMat = scene->mMaterials[i];
            String name = baseName + "_" + std::to_string(i) + Assets::materialExtension().string();
            bool wasAlreadyCreated;
            M<Material> mat = resourceManager()->create<Material>( name, wasAlreadyCreated );
            if ( wasAlreadyCreated )
            {
                LOGW("Material with name %s was already imported. Values of already imported material are taken.", name.c_str());
                materials.emplace_back( mat );
                continue; // Skip updating this material
            }
            aiAddSamplerFromModel( aiMat, mat.get(), aiTextureType_DIFFUSE );
            aiAddSamplerFromModel( aiMat, mat.get(), aiTextureType_SPECULAR );
            aiAddSamplerFromModel( aiMat, mat.get(), aiTextureType_AMBIENT );
            aiAddSamplerFromModel( aiMat, mat.get(), aiTextureType_EMISSIVE );
            aiAddSamplerFromModel( aiMat, mat.get(), aiTextureType_NORMALS );
        }
    }

#endif

    ModelImporter* g_ModelImporter {};
    ModelImporter* modelImporter() { return CreateOnce(g_ModelImporter); }
    void deleteModelImporter() { delete g_ModelImporter; g_ModelImporter=nullptr; }

}
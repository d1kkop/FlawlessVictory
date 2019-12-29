#pragma once
#include "../Core/Common.h"
#if FV_ASSIMP
    #include <assimp/cimport.h>
    #include <assimp/scene.h>
    #include <assimp/postprocess.h>
    #include <assimp/Importer.hpp>
    #if _MSC_VER
    #pragma comment(lib, "../3rdParty/Assimp/lib/x64/assimp-vc140-mt.lib")
    #endif
#endif

namespace fv
{
    class Material;
    struct Submesh;
    struct ResourceToLoad;

    struct MeshImportSettings
    {
        bool keepInRam = false;

        bool read(const Path& path);
        bool write(const Path& path);
        void serialize(class TextSerializer& serializer);
    };

    class ModelImporter
    {
    public:
        static bool writeBinary(const Path& path, const Vector<Submesh>& submeshes);
        static bool loadBinary(const Path& path, Vector<Submesh>& submeshes);

        bool reimport(const Path& path, const MeshImportSettings& settings, Vector<Submesh>& submeshes, Vector<M<Material>>& materials);

    #if FV_ASSIMP
        void loadSubmeshes(Vector<Submesh>& submeshes, const aiScene* scene);
        void loadMaterials(const String& baseName, Vector<M<Material>>& materials, const aiScene* scene);
        static enum class Binding aiTextureTypeToBinding( enum aiTextureType type );
        static void aiAddSamplerFromModel( const aiMaterial* aiMat, Material* mat, aiTextureType texType );
        static const char* aiPropertyNameToDefaultName( const String& aiName );
    #endif

    };


    FV_DLL ModelImporter* modelImporter();
    FV_DLL void deleteModelImporter();
}
#pragma once
#include "Mesh.h"
#include "../Core/Object.h"
#include "../Core/Common.h"

namespace fv
{
    class Resource;

    enum class PatchType
    {
        Texture2DData,
        ShaderCode,
        MeshData
    };

    // Instead of subclassing this. Make one 'big' patch object to avoid massive memory fragmentation by different types.
    // Patches are reycled!
    class Patch: public Object
    {
    public:
        void applyPatch();
        void submit(); // Same as PatchManager()->submiPatch( this );

        PatchType patchType;
        u64 graphic = -1;
        M<Resource> resource;
        union
        {
            struct
            {
                u32 width, height, depth;
                ImageFormat imgFormat;
            }; /* Texture2 or 3D */
            struct 
            {
            }; /* MeshData */
        };
        Vector<Submesh> hostMeshes; // In case importer wants to keep in host memory
        Vector<u64> submeshes; // For Meshdata
        Vector<M<Material>> materials;

    private:
        FV_MO void applyTexture2DLoad();
        FV_MO void applyShaderCode();
        FV_MO void applyMeshData();
    };
}
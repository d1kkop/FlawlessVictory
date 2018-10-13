#pragma once
#include "../Core/Object.h"
#include "../Core/Common.h"
#include "../Render/GraphicResource.h"

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
        ~Patch();
        void applyPatch();
        void submit(); // Same as PatchManager()->submiPatch( this );

        PatchType patchType;
        GraphicResource* graphic {};
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
                u32 numVertices, numIndices;
            }; /* MeshData */
        };
        Vector<GraphicResource*> submeshes; // For Meshdata

    private:
        FV_MO void applyTexture2DLoad();
        FV_MO void applyShaderCode();
        FV_MO void applyMeshData();
    };
}
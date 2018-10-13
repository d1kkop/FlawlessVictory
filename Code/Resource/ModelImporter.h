#pragma once
#include "../Core/Common.h"
#include "Mesh.h"

namespace fv
{
    struct ResourceToLoad;

    class ModelImporter
    {
    public:
        static Path replaceWithBinaryExtension( const Path& path );
        static bool writeBinary(const Path& path, const Vector<Submesh>& submeshes);
        static bool loadBinary(const Path& path, Vector<Submesh>& submeshes);

        bool reimport(const Path& path, Vector<Submesh>& submeshes);
    };


    FV_DLL ModelImporter* modelImporter();
    FV_DLL void deleteModelImporter();
}
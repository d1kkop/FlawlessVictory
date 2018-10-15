#pragma once
#include "../Core/Common.h"

namespace fv
{
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

        bool reimport(const Path& path, const MeshImportSettings& settings, Vector<Submesh>& submeshes);
    };


    FV_DLL ModelImporter* modelImporter();
    FV_DLL void deleteModelImporter();
}
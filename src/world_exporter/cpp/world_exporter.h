//
// Date       : 11/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_WORLD_EXPORTER_H
#define WORLD_EXPORTER_WORLD_EXPORTER_H

#include "pyunrealsdk/pch.h"

namespace world_exporter {

namespace fs = std::filesystem;
using namespace unrealsdk;
using namespace unrealsdk::unreal;

class WorldExporter {
   private:
    fs::path m_RootDir;
    std::vector<UObject*> m_Worlds;
    std::vector<UObject*> m_Terrains;
    std::vector<UObject*> m_StaticMeshComponents;

   public:
    void export_world(const fs::path& dest);

   private:
    void reset();
    void collect_exports_from_scene();
    void export_static_meshes();
};

}  // namespace world_exporter

#endif

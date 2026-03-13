//
// Date       : 11/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_WORLD_EXPORTER_H
#define WORLD_EXPORTER_WORLD_EXPORTER_H

#include "pyunrealsdk/pch.h"

#include "unrealsdk/unreal/classes/uclass.h"
#include "tinygltf/tiny_gltf.h"

namespace world_exporter {

namespace fs = std::filesystem;
using namespace unrealsdk;
using namespace unrealsdk::unreal;

namespace helpers {
struct UStaticMesh;
struct StaticMeshComponent;
}

class WorldExporter {
   private:
    fs::path m_RootDir;
    tinygltf::Model m_TheModel;
    tinygltf::Scene m_TheScene;

   private:
    std::vector<UObject*> m_Worlds;
    std::vector<UObject*> m_Terrains;
    std::vector<UObject*> m_StaticMeshComponents;
    std::unordered_map<uintptr_t, int> m_MeshMap; // TODO: not exactly 'unique' across levels

   private:
    static UClass* world_class;
    static UClass* terrain_class;
    static UClass* static_mesh_comp_class;
    static UClass* static_mesh_collection_class;

   public:
    void export_world(const fs::path& dest);

   private:
    void reset();
    void collect_exports_from_scene();
    void export_static_meshes();
    void export_static_mesh(const fs::path& mesh_path, helpers::UStaticMesh* mesh);
    void export_static_mesh_components();
    void export_static_mesh_component(helpers::StaticMeshComponent* comp);
};

}  // namespace world_exporter

#endif

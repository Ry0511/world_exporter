//
// Date       : 11/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/world_exporter.h"

#include "world_exporter/cpp/helpers.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/properties/zboolproperty.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"

namespace world_exporter {

using namespace helpers;

UClass* WorldExporter::world_class{nullptr};
UClass* WorldExporter::terrain_class{nullptr};
UClass* WorldExporter::static_mesh_comp_class{nullptr};
UClass* WorldExporter::static_mesh_collection_class{nullptr};

void WorldExporter::reset() {
    m_Worlds.clear();
    m_Terrains.clear();
    m_StaticMeshComponents.clear();
}

void WorldExporter::export_world(const fs::path& dest) {
    reset();
    m_RootDir = dest;
    if (auto parent = m_RootDir.parent_path(); !fs::is_directory(parent)) {
        throw std::runtime_error{std::format("parent directory must exist at {}", parent.string())};
    }
    fs::create_directory(m_RootDir);

    world_class = find_class(L"World"_fn);
    terrain_class = find_class(L"Terrain"_fn);
    static_mesh_comp_class = find_class(L"StaticMeshComponent"_fn);
    static_mesh_collection_class = find_class(L"StaticMeshCollectionActor"_fn);

    // default material at index 0
    tinygltf::Material mat{};
    mat.pbrMetallicRoughness.baseColorFactor = {1.0F, 1.0F, 1.0F, 1.0F};
    mat.doubleSided = true;
    m_TheModel.materials.push_back(mat);
    m_TheModel.asset.version = "2.0";
    m_TheModel.asset.generator = "tinygltf";

    collect_exports_from_scene();
    export_static_meshes();
    export_static_mesh_components();

    m_TheModel.scenes.push_back(m_TheScene);

    tinygltf::TinyGLTF gltf{};
    bool ok = gltf.WriteGltfSceneToFile(
        &m_TheModel,
        (dest / "export.gltf").string(),
        false,  // embedImages
        false,  // embedBuffers
        true,   // prettyPrint
        false   // writeBinary
    );
    if (!ok) {
        LOG(ERROR, "failed to write the scene to a .gltf file");
    }
}

void WorldExporter::collect_exports_from_scene() {
    const GObjects& gobj = unrealsdk::gobjects();
    std::vector<UObject*> roots{};

    for (size_t i = 0; i < gobj.size(); i++) {
        UObject* obj = gobj.obj_at(i);
        if (
            obj == nullptr
            || (obj->ObjectFlags() & (0x400 | 0x200)) != 0
        ) {
            continue;
        }

        // parent/outer of StaticMeshComponents
        if (obj->is_instance(static_mesh_collection_class)) {
            roots.emplace_back(obj);

        } else if (obj->is_instance(terrain_class)) {
            m_Terrains.emplace_back(obj);

        } else if (obj->is_instance(world_class)) {
            m_Worlds.emplace_back(obj);
        }
    }

    auto is_child_of_known_root = [&roots](UObject* obj) -> bool {
        const UObject* outer = obj->Outer();
        return std::ranges::any_of(
            roots.begin(),
            roots.end(),
            [&outer](const UObject* obj) { return obj == outer; }
        );
    };

    for (size_t i = 0; i < gobj.size(); i++) {
        UObject* obj = gobj.obj_at(i);
        if (
            obj == nullptr
            || (obj->ObjectFlags() & (0x400 | 0x200)) != 0
            || obj->Outer() == nullptr
            || !obj->is_instance(static_mesh_comp_class)
            || !is_child_of_known_root(obj)
        ) {
            continue;
        }
        m_StaticMeshComponents.emplace_back(obj);
    }
}

}  // namespace world_exporter

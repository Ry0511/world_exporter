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

namespace {
UClass* world_class{nullptr};
UClass* terrain_class{nullptr};
UClass* static_mesh_comp_class{nullptr};
UClass* static_mesh_collection_class{nullptr};
}  // namespace

void WorldExporter::reset() {
    m_Worlds.clear();
    m_Terrains.clear();
    m_StaticMeshComponents.clear();
}

void WorldExporter::export_world(const fs::path& dest) {
    reset();
    m_RootDir = dest;
    // m_AssetDir = dest / "_a";

    world_class = find_class(L"World"_fn);
    terrain_class = find_class(L"Terrain"_fn);
    static_mesh_comp_class = find_class(L"StaticMeshComponent"_fn);
    static_mesh_collection_class = find_class(L"StaticMeshCollectionActor"_fn);

    collect_exports_from_scene();

    LOG(
        INFO,
        "finished collecting exports worlds={}, terrains={}, meshes={}",
        m_Worlds.size(),
        m_Terrains.size(),
        m_StaticMeshComponents.size()
    );

    LOG(INFO, "Export Worlds");
    for (const auto* obj : m_Worlds) {
        LOG(INFO, " - {}", obj->get_path_name());
    }

    LOG(INFO, "Export Terrains");
    for (const auto* obj : m_Terrains) {
        LOG(INFO, " - {}", obj->get_path_name());
    }

    const auto* static_mesh_prop = static_mesh_comp_class->find_prop_and_validate<ZObjectProperty>(L"StaticMesh"_fn);
    // const auto* local_to_world_mat = static_mesh_comp_class->find_prop_and_validate<ZStructProperty>(L"_LocalToWorld"_fn);
    // const auto* is_hidden = static_mesh_comp_class->find_prop_and_validate<ZBoolProperty>(L"HiddenGame"_fn);

    LOG(INFO, "Export Meshes");
    std::unordered_set<std::wstring> unique_meshes{};
    for (const auto* obj : m_StaticMeshComponents) {
        LOG(INFO, " - {}", obj->get_path_name());

        auto* mesh_obj = get_property(static_mesh_prop, 0, reinterpret_cast<uintptr_t>(obj));
        auto* mesh = reinterpret_cast<UStaticMesh*>(mesh_obj);
        if (mesh != nullptr) {
            unique_meshes.insert(mesh_obj->get_path_name());
        }
    }

    LOG(INFO, "Unique Static Meshes");
    for (const auto& name : unique_meshes) {
        LOG(INFO, " - {}", name);
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

//
// Date       : 13/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "pyunrealsdk/pch.h"

#include "world_exporter/cpp/world_exporter.h"
#include "world_exporter/cpp/helpers.h"

#include "unrealsdk/unreal/properties/zobjectproperty.h"
#include "unrealsdk/unreal/properties/zstructproperty.h"
#include "unrealsdk/unreal/properties/zboolproperty.h"

namespace world_exporter {

using namespace helpers;

namespace {
ZObjectProperty* prop_static_mesh{nullptr};           // StaticMesh
ZStructProperty* prop_local_to_world{nullptr};        // _LocalToWorld
ZStructProperty* prop_local_to_world_scale{nullptr};  // _LocalToWorldScale
}  // namespace

void WorldExporter::export_static_mesh_components() {
    prop_static_mesh = static_mesh_comp_class->find_prop_and_validate<ZObjectProperty>(L"StaticMesh"_fn);
    prop_local_to_world = static_mesh_comp_class->find_prop_and_validate<ZStructProperty>(L"_LocalToWorld"_fn);
    prop_local_to_world_scale = static_mesh_comp_class->find_prop_and_validate<ZStructProperty>(L"_LocalToWorldScale"_fn);
    for (auto* obj : m_StaticMeshComponents) {
        export_static_mesh_component(reinterpret_cast<StaticMeshComponent*>(obj));
    }
}

void WorldExporter::export_static_mesh_component(StaticMeshComponent* comp) {
    WrappedStruct wrap = get_property(prop_local_to_world, 0, reinterpret_cast<uintptr_t>(comp));
    auto* mat = reinterpret_cast<FMatrix*>(wrap.base.get());

    UObject* mesh = get_property(prop_static_mesh, 0, reinterpret_cast<uintptr_t>(comp));

    tinygltf::Node node{};
    node.mesh = m_MeshMap[reinterpret_cast<uintptr_t>(mesh)];
    node.matrix.reserve(16);
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            node.matrix.emplace_back(static_cast<double>(mat->M[i][j]));
        }
    }

    m_TheScene.nodes.push_back(m_TheModel.nodes.size());
    m_TheModel.nodes.push_back(node);
}

}  // namespace world_exporter

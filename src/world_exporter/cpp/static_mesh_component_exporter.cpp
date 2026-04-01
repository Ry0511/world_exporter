//
// Date       : 13/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/pch.h"

#include "world_exporter/cpp/world_exporter.h"
#include "world_exporter/cpp/util/static_mesh.h"

#include "unrealsdk/unreal/properties/zobjectproperty.h"
#include "unrealsdk/unreal/properties/zstructproperty.h"
#include "unrealsdk/unreal/properties/zboolproperty.h"

namespace world_exporter {

using namespace helpers;

namespace {
ZObjectProperty* prop_static_mesh{nullptr};  // StaticMesh
ZFloatProperty* prop_scale{nullptr};         // Scale
ZStructProperty* prop_scale3d{nullptr};      // Scale3D
UFunction* fn_get_position{nullptr};         // GetPosition
UFunction* fn_get_rotation{nullptr};         // GetRotation

// NOLINTBEGIN(*-pro-bounds-constant-array-index,*-math-missing-parentheses)

struct AngleTable {
    static constexpr int ANGLE_SHIFT = 2;
    static constexpr int ANGLE_BITS = 14;
    static constexpr int NUM_ANGLES = 16384;
    std::array<float, NUM_ANGLES> lut{};
    AngleTable() {
        constexpr auto PI = glm::pi<float>();
        for (int i = 0; i < NUM_ANGLES; i++) {
            lut[i] = std::sinf(static_cast<float>(i) * 2.0F * PI / static_cast<float>(NUM_ANGLES));
        }
    }
    float sin_tab(int i) const noexcept {
        return lut[((i >> ANGLE_SHIFT) & (NUM_ANGLES - 1))];
    }
    float cos_tab(int i) const noexcept {
        return lut[(((i + 16384) >> ANGLE_SHIFT) & (NUM_ANGLES - 1))];
    }
};

AngleTable lut{};

glm::mat4 create_rot_matrix(const FRotator& rot) {
    glm::mat4 mat{1.0F};

    float SR = lut.sin_tab(rot.Roll);
    float SP = lut.sin_tab(rot.Pitch);
    float SY = lut.sin_tab(rot.Yaw);
    float CR = lut.cos_tab(rot.Roll);
    float CP = lut.cos_tab(rot.Pitch);
    float CY = lut.cos_tab(rot.Yaw);

    mat[0][0] = CP * CY;
    mat[0][1] = CP * SY;
    mat[0][2] = SP;

    mat[1][0] = SR * SP * CY - CR * SY;
    mat[1][1] = SR * SP * SY + CR * CY;
    mat[1][2] = -SR * CP;

    mat[2][0] = -(CR * SP * CY + SR * SY);
    mat[2][1] = CY * SR - CR * SP * SY;
    mat[2][2] = CR * CP;

    return mat;
}

// NOLINTEND(*-pro-bounds-constant-array-index,*-math-missing-parentheses)

}  // namespace

void WorldExporter::export_static_mesh_components() {
    prop_static_mesh = static_mesh_comp_class->find_prop_and_validate<ZObjectProperty>(L"StaticMesh"_fn);
    prop_scale = static_mesh_comp_class->find_prop_and_validate<ZFloatProperty>(L"Scale"_fn);
    prop_scale3d = static_mesh_comp_class->find_prop_and_validate<ZStructProperty>(L"Scale3D"_fn);
    fn_get_position = static_mesh_comp_class->find_func_and_validate(L"GetPosition"_fn);
    fn_get_rotation = static_mesh_comp_class->find_func_and_validate(L"GetRotation"_fn);

    for (auto* obj : m_StaticMeshComponents) {
        export_static_mesh_component(reinterpret_cast<StaticMeshComponent*>(obj));
    }
}

void WorldExporter::export_static_mesh_component(StaticMeshComponent* comp) {
    auto* obj = reinterpret_cast<UObject*>(comp);
    UObject* mesh = get_property(prop_static_mesh, 0, reinterpret_cast<uintptr_t>(comp));

    auto it = m_MeshMap.find(reinterpret_cast<uintptr_t>(mesh));
    if (it == m_MeshMap.end()) {
        LOG(
            WARNING,
            "could not export mesh instance {} because the mesh has not been exported",
            obj->get_path_name()
        );
        return;
    }

    tinygltf::Node node{};
    node.mesh = it->second;

    // Apply the translation
    auto wpos = obj->get<UFunction, BoundFunction>(fn_get_position).call<ZStructProperty>();
    const auto& pos = *reinterpret_cast<FVector*>(wpos.base.get());
    node.translation = {-(pos.Y * 0.01F), pos.Z * 0.01F, pos.X * 0.01F};

    // Apply the scaling
    auto scale = get_property(prop_scale, 0, reinterpret_cast<uintptr_t>(obj));
    auto wscale3d = get_property(prop_scale3d, 0, reinterpret_cast<uintptr_t>(obj));
    const auto& scale3d = *reinterpret_cast<FVector*>(wscale3d.base.get());
    glm::vec3 vscale{(scale3d.X * scale), (scale3d.Y * scale), (scale3d.Z * scale)};
    node.scale = {vscale.y, vscale.z, vscale.x};

    // Apply the rotation
    auto wrot = obj->get<UFunction, BoundFunction>(fn_get_rotation).call<ZStructProperty>();
    const auto& rot = *reinterpret_cast<FRotator*>(wrot.base.get());

    // TODO: This does not account for the effects that negative scale has on the final rotated
    //  output so this only going to work for objects whose scale is greater than zero.
    glm::mat4 basis{
        glm::vec4{+0, +0, +1, +0},
        glm::vec4{-1, +0, +0, +0},
        glm::vec4{+0, +1, +0, +0},
        glm::vec4{+0, +0, +0, +1},
    };
    glm::mat4 rot_mat = create_rot_matrix(rot);
    rot_mat = basis * rot_mat * glm::inverse(basis);
    glm::quat q = glm::normalize(glm::quat_cast(rot_mat));
    node.rotation = {q.x, q.y, q.z, q.w};

    m_TheScene.nodes.push_back(static_cast<int>(m_TheModel.nodes.size()));
    m_TheModel.nodes.push_back(node);
}

}  // namespace world_exporter

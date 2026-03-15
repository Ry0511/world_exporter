//
// Date       : 13/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "pyunrealsdk/pch.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include "world_exporter/cpp/world_exporter.h"
#include "world_exporter/cpp/helpers.h"

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
    glm::mat4 mat{};
    float SR = lut.sin_tab(rot.Roll);
    float SP = lut.sin_tab(rot.Pitch);
    float SY = lut.sin_tab(rot.Yaw);
    float CR = lut.cos_tab(rot.Roll);
    float CP = lut.cos_tab(rot.Pitch);
    float CY = lut.cos_tab(rot.Yaw);

    mat[0][0] = CP * CY;
    mat[1][0] = CP * SY;
    mat[2][0] = SP;
    mat[3][0] = 0.0F;

    mat[0][1] = SR * SP * CY - CR * SY;
    mat[1][1] = SR * SP * SY + CR * CY;
    mat[2][1] = -SR * CP;
    mat[3][1] = 0.0F;

    mat[0][2] = -(CR * SP * CY + SR * SY);
    mat[1][2] = CY * SR - CR * SP * SY;
    mat[2][2] = CR * CP;
    mat[3][2] = 0.0F;

    mat[0][3] = 0.0F;
    mat[1][3] = 0.0F;
    mat[2][3] = 0.0F;
    mat[3][3] = 1.0F;

    return mat;
}

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

    tinygltf::Node node{};
    node.mesh = m_MeshMap[reinterpret_cast<uintptr_t>(mesh)];

    // Apply the translation
    auto wpos = obj->get<UFunction, BoundFunction>(fn_get_position).call<ZStructProperty>();
    const auto& pos = *reinterpret_cast<FVector*>(wpos.base.get());
    node.translation = {-(pos.Y * 0.01F), pos.Z * 0.01F, pos.X * 0.01F};

    // Apply the scaling
    auto scale = get_property(prop_scale, 0, reinterpret_cast<uintptr_t>(obj));
    auto wscale3d = get_property(prop_scale3d, 0, reinterpret_cast<uintptr_t>(obj));
    const auto& scale3d = *reinterpret_cast<FVector*>(wscale3d.base.get());
    node.scale = {(scale3d.Y * scale), (scale3d.Z * scale), (scale3d.X * scale)};

    auto wrot = obj->get<UFunction, BoundFunction>(fn_get_rotation).call<ZStructProperty>();
    const auto& rot = *reinterpret_cast<FRotator*>(wrot.base.get());

    {
        glm::mat4 ue_rot = create_rot_matrix(rot);
        glm::mat4 C = glm::mat4(
            glm::vec4{0, 0, 1, 0},
            glm::vec4{1, 0, 0, 0},
            glm::vec4{0, 1, 0, 0},
            glm::vec4{0, 0, 0, 1}
        );
        glm::mat4 gltf = C * ue_rot * glm::transpose(C);
        glm::quat q = glm::normalize(glm::quat_cast(gltf));
        node.rotation = {-q.x, q.y, q.z, q.w};
    }

    m_TheScene.nodes.push_back(m_TheModel.nodes.size());
    m_TheModel.nodes.push_back(node);
}

}  // namespace world_exporter

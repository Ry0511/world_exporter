//
// Date       : 06/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "pyunrealsdk/pch.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/properties/zboolproperty.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "world_exporter/cpp/helpers.h"

namespace fs = std::filesystem;

namespace {

using namespace unrealsdk;
using namespace unrealsdk::unreal;
using namespace world_exporter::helpers;

void export_world(const fs::path& /*dest*/, const std::wstring& obj_path) {
    auto* cls = unreal::find_class(L"World"_fn);
    auto* obj = find_object(cls, obj_path);
    auto* actual = reinterpret_cast<UWorld*>(obj);
    LOG(INFO, "Model {:p}, {}", (void*)actual, obj_path);
}

void export_static_mesh(const fs::path& dest, const std::wstring& obj_path) {
    auto* cls = unreal::find_class(L"StaticMesh"_fn);
    auto* obj = find_object(cls, obj_path);
    auto* actual = reinterpret_cast<UStaticMesh*>(obj);
    LOG(INFO, "Model {:p}, {}", (void*)actual, obj_path);

    if (actual->LodModels.size() == 0) {
        LOG(INFO, "no model info could be found for '{}'", obj_path);
        return;
    }

    std::ofstream out{dest};
    auto& model = actual->LodModels[0];

    {  // write all the vertices of the mesh
        auto& positions = model->PositionVertexBuffer;
        auto* data = positions.VertexData->data();
        auto stride = positions.VertexData->stride();
        for (size_t i = 0; i < positions.NumVertices; ++i) {
            auto* pos = reinterpret_cast<FVector*>(data + (i * stride));
            out << "v " << pos->X << " " << pos->Y << " " << pos->Z << "\n";
        }
    }

    {  // write all the faces
        for (size_t i = 0; i < model->IndexBuffer.Indices.size(); i += 3) {
            out << "f " << (model->IndexBuffer.Indices[i] + 1)
                << " " << (model->IndexBuffer.Indices[i + 1] + 1)
                << " " << (model->IndexBuffer.Indices[i + 2] + 1) << "\n";
        }
    }
}

void export_static_meshes(const fs::path& dest) {
    const auto outer_name = L"TheWorld"_fn;
    const auto parent_type = L"StaticMeshCollectionActor"_fn;
    const auto* mesh_cls = unreal::find_class(L"StaticMeshComponent"_fn);
    const auto* static_mesh_prop = mesh_cls->find_prop_and_validate<ZObjectProperty>(L"StaticMesh"_fn);
    const auto* local_to_world_mat = mesh_cls->find_prop_and_validate<ZStructProperty>(L"_LocalToWorld"_fn);
    const auto* is_hidden = mesh_cls->find_prop_and_validate<ZBoolProperty>(L"HiddenGame"_fn);
    const GObjects& gobj = gobjects();

    struct MeshExportInfo {
        StaticMeshComponent* TheMesh;
        size_t BaseIndex;
        bool Valid{true};
    };
    std::vector<MeshExportInfo> meshes{};

    for (size_t i = 0; i < gobj.size(); ++i) {
        UObject* obj = gobj.obj_at(i);
        if (
            obj == nullptr
            || (obj->ObjectFlags() & (0x400 | 0x200)) != 0
            || obj->Outer() == nullptr
            || !obj->is_instance(mesh_cls)
        ) {
            continue;
        }

        UObject* outer = obj->Outer();
        while (outer != nullptr) {
            if (outer->Name() == outer_name) {
                break;
            }
            outer = outer->Outer();
        }

        if (outer == nullptr || obj->Outer()->Class()->Name() != parent_type) {
            continue;
        }

        auto hidden = get_property(is_hidden, 0, reinterpret_cast<uintptr_t>(outer));
        if (!hidden) {
            meshes.emplace_back(reinterpret_cast<StaticMeshComponent*>(obj), 0);
        }
    }

    std::ofstream out{dest};
    size_t base_index = 0;

    // first pass gets all the vertex positions
    for (auto& info : meshes) {
        auto addr = reinterpret_cast<uintptr_t>(info.TheMesh);
        auto* mesh = reinterpret_cast<UStaticMesh*>(get_property(static_mesh_prop, 0, addr));

        if (mesh == nullptr || mesh->LodModels.size() == 0) {
            LOG(INFO, "no model info could be found for '{}'", info.TheMesh->Name);
            continue;
        }

        auto& model = mesh->LodModels[0];

        auto& positions = model->PositionVertexBuffer;
        auto* data = positions.VertexData->data();
        auto stride = positions.VertexData->stride();

        if (data == nullptr) {
            LOG(ERROR, "could not export {} as the data pointer is null", mesh->Name);
            info.Valid = false;
            continue;
        }

        auto* obj = reinterpret_cast<UObject*>(info.TheMesh);
        WrappedStruct m = get_property(local_to_world_mat, 0, reinterpret_cast<uintptr_t>(obj));
        const auto& mat = *reinterpret_cast<FMatrix*>(m.base.get());

        for (size_t i = 0; i < positions.NumVertices; ++i) {
            const auto& pos = *reinterpret_cast<FVector*>(data + (i * stride));
            FVector p = transform_point(mat, pos);
            out << "v " << p.X << " " << p.Y << " " << p.Z << "\n";
        }
        info.BaseIndex = base_index;
        base_index += positions.NumVertices;
    }

    // 2nd pass dumps all the indices
    for (auto& info : meshes) {
        auto addr = reinterpret_cast<uintptr_t>(info.TheMesh);
        auto* mesh = reinterpret_cast<UStaticMesh*>(get_property(static_mesh_prop, 0, addr));

        if (mesh == nullptr || mesh->LodModels.size() == 0) {
            LOG(INFO, "no model info could be found for '{}'", info.TheMesh->Name);
            continue;
        }

        if (!info.Valid) {
            continue;
        }

        auto& model = mesh->LodModels[0];
        for (size_t i = 0; i < model->IndexBuffer.Indices.size(); i += 3) {
            out << "f " << (info.BaseIndex + model->IndexBuffer.Indices[i] + 1)
                << " " << (info.BaseIndex + model->IndexBuffer.Indices[i + 1] + 1)
                << " " << (info.BaseIndex + model->IndexBuffer.Indices[i + 2] + 1) << "\n";
        }
    }
}

}  // namespace

PYBIND11_MODULE(world_exporter, m) {
    m.def(
        "export_world",
        &export_world,
        "dest"_a,
        "obj_path"_a
    );
    m.def(
        "export_static_mesh",
        &export_static_mesh,
        "dest"_a,
        "obj_path"_a
    );
    m.def(
        "export_static_meshes",
        &export_static_meshes,
        "dest"_a
    );
}
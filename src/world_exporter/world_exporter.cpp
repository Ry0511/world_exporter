//
// Date       : 06/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "pyunrealsdk/pch.h"
#include "unrealsdk/unreal/find_class.h"
#include "world_exporter/helpers.h"

namespace fs = std::filesystem;

namespace {

using namespace unrealsdk;
using namespace unrealsdk::unreal;
using namespace world_exporter::helpers;

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

}  // namespace

PYBIND11_MODULE(world_exporter, m) {
    m.def(
        "export_static_mesh",
        &export_static_mesh,
        "dest"_a,
        "obj_path"_a
    );
}
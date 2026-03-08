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

void export_static_mesh(const fs::path& /*dest*/, const std::wstring& obj_path) {
    auto* cls = unreal::find_class(L"StaticMesh"_fn);
    auto* obj = find_object(cls, obj_path);
    auto* actual = reinterpret_cast<UStaticMesh*>(obj);
    LOG(INFO, "Model = {:p}", (void*)actual);

    size_t lod_index = 0;
    for (const auto& model : actual->LodModels) {
        if (model->VertexBuffer.Stride != sizeof(FVector)) {
            LOG(INFO, "Vertex data for model {:p} is likely not 3 floats", (void*)model);
            continue;
        }
        LOG(INFO, "LOD ~ {:>2}", lod_index++);
        auto* data = reinterpret_cast<FVector*>(model->VertexBuffer.VertexData);
        for (size_t i = 0; i < model->VertexBuffer.NumVertices; ++i) {
            const auto& elem = data[i];
            LOG(INFO, "{:>3} {:.3f},{:.3f},{:.3f}", i, elem.X, elem.Y, elem.Z);
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
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
    LOG(INFO, "Model {:p}, {}", (void*)actual, obj_path);

    size_t lod_index = 0;
    for (const auto& model : actual->LodModels) {
        auto& positions = model->PositionVertexBuffer;
        auto* data = positions.VertexData->data();
        auto stride = positions.VertexData->stride();

        LOG(INFO, "  LOD[{:>}] - {}, {:p}", lod_index++, stride, (void*)data);

        if (data == nullptr) {
            LOG(INFO, "   - data pointer is null");
            continue;
        }

        for (size_t i = 0; i < positions.NumVertices; ++i) {
            auto* pos = reinterpret_cast<FVector*>(data + (i * stride));
            LOG(INFO, "   - {:.3f},{:.3f},{:.3f}", pos->X, pos->Y, pos->Z);
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
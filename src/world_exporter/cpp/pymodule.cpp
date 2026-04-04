//
// Date       : 06/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/pch.h"
#include "world_exporter/cpp/world_exporter.h"
#include "world_exporter/cpp/exporter/texture_exporter.h"

#include "unrealsdk/unreal/find_class.h"

namespace {

using namespace unrealsdk;
using namespace unrealsdk::unreal;
using namespace world_exporter::helpers;

void export_world(const fs::path& dest) {
    world_exporter::WorldExporter exporter{};
    exporter.export_world(dest);
}

void impl_testing(const std::wstring& obj_path) {
    auto* tex2d_class = find_class(L"Texture2D"_fn);
    unreal::UObject* obj = find_object(tex2d_class, obj_path);

    if (obj == nullptr) {
        LOG(INFO, L" > failed to find {}", obj_path);
        return;
    }

    if (obj->Class() != tex2d_class) {
        LOG(
            INFO,
            " > object found is of class {} but {} is required",
            obj->Class()->Name(),
            tex2d_class->Name()
        );
        return;
    }

    world_exporter::TextureExporter texture_exporter{};
    if (texture_exporter.export_texture(obj)) {
        auto& info = texture_exporter.export_info();
        if (info.is_srgb) {
            info.to_linear();
        }
        static size_t index{0};
        fs::path out_path{
            "G:/Dev/git_borderlands/world_exporter/src/world_exporter/py/exports/img/"
            + std::to_string(index++) + ".ppm"
        };
        info.write_to(out_path);
    }
}

}  // namespace

PYBIND11_MODULE(world_exporter, m) {
    m.def(
        "export_world",
        &export_world,
        "dest"_a
    );
    m.def(
        "testing",
        [](const std::wstring& obj_path) { impl_testing(obj_path); },
        "obj_path"_a
    );
}
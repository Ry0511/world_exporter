//
// Date       : 06/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/pch.h"
#include "world_exporter/cpp/world_exporter.h"
#include "world_exporter/cpp/util/materials.h"
#include "world_exporter/cpp/util/texture.h"

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

    using namespace world_exporter::helpers;
    auto* prop_resource = tex2d_class->find_prop_and_validate<ZStructProperty>(L"Resource"_fn);
    auto wres = get_property(prop_resource, 0, reinterpret_cast<uintptr_t>(obj));
    auto* res = *reinterpret_cast<FTexture2DResource**>(wres.base.get());

    if (res == nullptr) {
        LOG(INFO, L"resource is null for {}", obj_path);
        return;
    }
    std::wstring owner = L"NULL";
    if (auto p = res->Owner) {
        owner = p->get_path_name();
    }

    try {
        auto img = export_texture_resource(res);

        std::string ansi_path = unrealsdk::utils::narrow(obj_path);
        std::ranges::transform(ansi_path.begin(), ansi_path.end(), ansi_path.begin(), ::tolower);
        fs::path out_path{
            "G:/Dev/git_borderlands/world_exporter/src/world_exporter/py/exports/img/"
            + ansi_path + ".ppm"
        };

        std::ofstream out{out_path, std::ios::binary | std::ios::trunc};
        out << "P6\n"
            << img.width << " " << img.height << "\n255\n";
        for (size_t i = 0; i < (img.width * img.height); ++i) {
            const uint8_t* px = img.data.get() + (i * 4);
            const uint8_t rgba[]{px[3], px[2], px[1], px[0]};
            out.write(reinterpret_cast<const char*>(rgba), 3);
        }

        LOG(INFO, "exported image {}x{} from {}", img.width, img.height, obj_path);
    } catch (const std::exception& err) {
        LOG(INFO, "failed to export texture ~ {}", err.what());
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
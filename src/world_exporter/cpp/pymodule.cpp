//
// Date       : 06/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "pyunrealsdk/pch.h"
#include "world_exporter/cpp/world_exporter.h"

namespace fs = std::filesystem;

namespace {

using namespace unrealsdk;
using namespace unrealsdk::unreal;
using namespace world_exporter::helpers;

void export_world(const fs::path& dest) {
    world_exporter::WorldExporter exporter{};
    exporter.export_world(dest);
}

}  // namespace

PYBIND11_MODULE(world_exporter, m) {
    m.def(
        "export_world",
        &export_world,
        "dest"_a
    );
}
//
// Date       : 04/04/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_MATERIAL_EXPORTER_H
#define WORLD_EXPORTER_MATERIAL_EXPORTER_H

#include "world_exporter/cpp/pch.h"
#include "world_exporter/cpp/exporter/texture_exporter.h"

namespace world_exporter {

// TODO: going for a very basic material exporter for now but in the future this will probably need
//  to be a bit more sophisticated
struct MaterialExportInfo {
    TextureExportInfo diffuse_texture;
    TextureExportInfo normal_texture;
    operator bool() const noexcept { return diffuse_texture || normal_texture; }
};

class MaterialExporter {
   private:
    MaterialExportInfo m_Export;

   public:
    const MaterialExportInfo& export_info() const noexcept { return m_Export; }
    bool export_material(unrealsdk::unreal::UObject* obj);
};

}  // namespace world_exporter

#endif

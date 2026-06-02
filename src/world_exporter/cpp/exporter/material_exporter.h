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

enum class BlendMode : uint8_t {
    Opaque = 0,
    Masked,
    Translucent,
    Additive,
    Modulate,
    SoftMasked,
    AlphaComposite,
    DitheredTranslucent,
};

struct MaterialExportInfo {
    TextureExportInfo diffuse_texture;
    TextureExportInfo normal_texture;
    TextureExportInfo emissive_texture;
    bool is_double_sided{false};              // var() bool TwoSided;
    float alpha_cutoff{0.5F};                 // var() float OpacityMaskClipValue;
    BlendMode blend_mode{BlendMode::Opaque};  // var() EngineTypes.EBlendMode BlendMode;
    operator bool() const noexcept { return diffuse_texture || normal_texture; }
};

class MaterialExporter {
   private:
    MaterialExportInfo m_Export;

   public:
    MaterialExportInfo& export_info() noexcept { return m_Export; }
    bool export_material(unrealsdk::unreal::UObject* obj);

   private:
    void extract_material_interface(unrealsdk::unreal::UObject* mat);
    void extract_material_instance_constant(unrealsdk::unreal::UObject* mat);
    void extract_material_props(unrealsdk::unreal::UObject* mat);
};

}  // namespace world_exporter

#endif

//
// Date       : 03/04/2026
// Project    : world_exporter
// Author     : -Ry
//

#ifndef WORLD_EXPORTER_TEXTURE_EXPORTER_H
#define WORLD_EXPORTER_TEXTURE_EXPORTER_H

#include "world_exporter/cpp/pch.h"
#include "world_exporter/cpp/util/texture.h"

namespace world_exporter {

// TODO: certain sampler parameters should also be exported
struct TextureExportInfo {
    static constexpr auto num_components = 4;
    std::unique_ptr<uint8_t[]> data{nullptr};  // RGBA literal bytes
    uint32_t width{0};
    uint32_t height{0};
    bool is_srgb{false};

    uint32_t size_in_bytes() const noexcept { return width * height * uint32_t{num_components}; }
    void to_linear() noexcept;
    void to_srgb() noexcept;
    void write_to(const fs::path& out) const;
};

class TextureExporter {
   private:
    TextureExportInfo m_Export;

   public:
    TextureExportInfo& export_info() noexcept { return m_Export; }
    const TextureExportInfo& export_info() const noexcept { return m_Export; }

    /**
     * Fills the internal transient buffer with the exported texture data (if exporting was successful)
     * @param texture the texture to export must be a Texture or Texture2D
     * @return true if the export was successful, false otherwise
     */
    bool export_texture(unrealsdk::unreal::UObject* texture);

   private:
    void extract_pixel_data_from_rhi(helpers::FD3D9Texture* rhi);
};

}  // namespace world_exporter

#endif

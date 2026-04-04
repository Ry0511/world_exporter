//
// Date       : 03/04/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/exporter/texture_exporter.h"

#include "unrealsdk/unreal/find_class.h"
#include "s3tc-dxt-decompression/s3tc.h"
#include "stb/stb_image_write.h"

namespace world_exporter {

////////////////////////////////////////////////////////////////////////////////
// | INTERNAL HELPERS |
////////////////////////////////////////////////////////////////////////////////

namespace {
using namespace unrealsdk;
using namespace unrealsdk::unreal;
using LookUpTable = std::array<uint8_t, 256>;
LookUpTable init_srgb_to_linear_lut() noexcept;
LookUpTable init_linear_to_srgb_lut() noexcept;
const LookUpTable srgb_to_linear_lut = init_srgb_to_linear_lut();
const LookUpTable linear_to_srgb_lut = init_linear_to_srgb_lut();

// these are the primary formats used in unreal engine
constexpr auto supported_formats = {
    D3DFMT_DXT1,
    D3DFMT_DXT5,
    D3DFMT_A8R8G8B8
};

bool is_supported_format(const D3DFORMAT& fmt) noexcept {
    return std::ranges::any_of(
        supported_formats,
        [&fmt](const auto& elem) {
            return elem == fmt;
        }
    );
}

void decompress_dxt1(TextureExportInfo& out, uint8_t* data);
void decompress_dxt5(TextureExportInfo& out, uint8_t* data);

}  // namespace

using namespace helpers;

////////////////////////////////////////////////////////////////////////////////
// | PUBLIC FUNCTIONS |
////////////////////////////////////////////////////////////////////////////////

void TextureExportInfo::write_to(const fs::path& out) const {
    std::string ext = out.extension().string();
    std::ranges::transform(ext, ext.begin(), ::tolower);
    if (ext == ".ppm") {
        std::ofstream ofs{out, std::ios::binary | std::ios::trunc};
        if (!ofs) {
            throw std::runtime_error{"failed to write .ppm file; " + out.filename().string()};
        }
        ofs << std::format("P6\n{} {}\n255\n", width, height);
        for (size_t i = 0; i < size_in_bytes(); i += 4) {
            const uint8_t* px = data.get() + i;
            ofs.write(reinterpret_cast<const char*>(px), 3);  // RGB
        }
    } else if (ext == ".png") {
        int res = stbi_write_png(
            out.string().c_str(),
            static_cast<int>(width),
            static_cast<int>(height),
            num_components,  // 4 components
            data.get(),
            static_cast<int>(width) * num_components
        );
        if (res == 0) {
            throw std::runtime_error{"failed to write .png file; " + out.string()};
        }
    } else if (ext == ".tga") {
        int res = stbi_write_tga(
            out.string().c_str(),
            static_cast<int>(width),
            static_cast<int>(height),
            num_components,
            data.get()
        );
        if (res == 0) {
            throw std::runtime_error{"failed to write .tga file; " + out.string()};
        }
    } else {
        throw std::runtime_error{"unsupported file format; " + out.filename().string()};
    }
}

// NOLINTBEGIN(*-pro-bounds-constant-array-index)

void TextureExportInfo::to_linear() noexcept {
    if (!is_srgb) {
        return;
    }
    is_srgb = false;
    for (uint32_t i = 0; i < size_in_bytes(); i += 4) {
        uint8_t* px = data.get() + i;
        px[0] = srgb_to_linear_lut[px[0]];
        px[1] = srgb_to_linear_lut[px[1]];
        px[2] = srgb_to_linear_lut[px[2]];
    }
}

void TextureExportInfo::to_srgb() noexcept {
    if (is_srgb) {
        return;
    }
    is_srgb = true;
    for (uint32_t i = 0; i < size_in_bytes(); i += 4) {
        uint8_t* px = data.get() + i;
        px[0] = linear_to_srgb_lut[px[0]];
        px[1] = linear_to_srgb_lut[px[1]];
        px[2] = linear_to_srgb_lut[px[2]];
    }
}

bool TextureExporter::export_texture(UObject* texture) {
    static auto texture_cls = find_class(L"Texture"_fn);
    static auto prop_resource = texture_cls->find_prop_and_validate<ZStructProperty>(L"Resource"_fn);

    if (texture == nullptr) {
        LOG(ERROR, "can not export null texture");
        return false;
    }

    if (!texture->is_instance(texture_cls)) {
        LOG(ERROR, "texture is not an instance of Texture; {}", texture->get_path_name());
        return false;
    }

    auto wres = get_property(prop_resource, 0, reinterpret_cast<uintptr_t>(texture));
    auto* res = *reinterpret_cast<FTexture2DResource**>(wres.base.get());

    if (res == nullptr || res->TextureRhi.Reference == nullptr) {
        LOG(ERROR, "Texture does not have a valid RHI resource; {}", texture->get_path_name());
        return false;
    }

    auto* rhi = res->TextureRhi.as<FD3D9Texture>();

    try {
        extract_pixel_data_from_rhi(rhi);
    } catch (const std::runtime_error& err) {
        LOG(ERROR, "failed to export texture {}", texture->get_path_name());
        LOG(ERROR, "with reason: {}", err.what());
    }

    return true;
}

void TextureExporter::extract_pixel_data_from_rhi(helpers::FD3D9Texture* rhi) {
    if (rhi->Ref == nullptr) {
        throw std::runtime_error{"RHI reference is null"};
    }

    //
    // Only downside to this approach is that we are limited to exporting what is being used to
    // render the scene. Would prefer to export the highest quality textures always, but for now this
    // will do since we only need the 'look and feel' not 1:1 output.
    //

    auto* rhi_ref = rhi->Ref;
    IDirect3DTexture9* texture{nullptr};
    HRESULT result{0};

    result = rhi_ref->QueryInterface(IID_IDirect3DTexture9, reinterpret_cast<void**>(&texture));
    if (FAILED(result) || texture == nullptr) {
        throw std::runtime_error{"underlying texture is not a Texture2D"};
    }

    D3DSURFACE_DESC desc;
    result = texture->GetLevelDesc(0, &desc);
    if (FAILED(result)) {
        texture->Release();
        throw std::runtime_error{"failed to obtain surface zero description"};
    }

    if (!is_supported_format(desc.Format)) {
        texture->Release();
        throw std::runtime_error{
            std::format("unsupported pixel format - {}", static_cast<int>(desc.Format))
        };
    }

    IDirect3DSurface9* surface{nullptr};
    result = texture->GetSurfaceLevel(0, &surface);
    if (FAILED(result)) {
        texture->Release();
        throw std::runtime_error{"failed to obtain surface zero"};
    }

    D3DLOCKED_RECT locked_rect;
    result = surface->LockRect(&locked_rect, nullptr, D3DLOCK_READONLY);
    if (FAILED(result)) {
        surface->Release();
        texture->Release();
        throw std::runtime_error{"failed to lock surface zero"};
    }

    // populate the internal buffer
    m_Export.width = desc.Width;
    m_Export.height = desc.Height;
    m_Export.is_srgb = rhi->bSRGB != 0;
    m_Export.data = std::make_unique<uint8_t[]>(m_Export.size_in_bytes());

    if (desc.Format == D3DFMT_DXT1) {
        decompress_dxt1(m_Export, static_cast<uint8_t*>(locked_rect.pBits));

    } else if (desc.Format == D3DFMT_DXT5) {
        decompress_dxt5(m_Export, static_cast<uint8_t*>(locked_rect.pBits));

    } else if (desc.Format == D3DFMT_A8R8G8B8) {
        auto* pbits = reinterpret_cast<uint8_t*>(locked_rect.pBits);
        for (uint32_t i = 0; i < m_Export.size_in_bytes(); i += 4) {
            auto* data = m_Export.data.get();
            // stored as packed ARGB which is BGRA bytewise in little endian; we want RGBA bytewise
            data[i + 0] = pbits[i + 2];  // R
            data[i + 1] = pbits[i + 1];  // G
            data[i + 2] = pbits[i + 0];  // B
            data[i + 3] = pbits[i + 3];  // A
        }
    }

    surface->UnlockRect();
    surface->Release();
    texture->Release();
}

////////////////////////////////////////////////////////////////////////////////
// | HELPERS IMPL |
////////////////////////////////////////////////////////////////////////////////

namespace {

LookUpTable init_srgb_to_linear_lut() noexcept {
    // following: https://en.wikipedia.org/wiki/SRGB#Transfer_function_(%22gamma%22)
    LookUpTable lut{};
    for (int i = 0; i < 256; ++i) {
        float linear = static_cast<float>(i) / 255.0F;
        linear = (linear <= 0.04045F)
                     ? linear / 12.92F
                     : std::powf((linear + 0.055F) / 1.055F, 2.4F);
        lut[i] = static_cast<uint8_t>(
            std::clamp(
                static_cast<int>((linear * 255.0F) + 0.5F),
                0,
                255
            )
        );
    }
    return lut;
}

LookUpTable init_linear_to_srgb_lut() noexcept {
    // following: https://en.wikipedia.org/wiki/SRGB#Transfer_function_(%22gamma%22)
    // using the inverse oetf variant
    LookUpTable lut{};
    for (int i = 0; i < 256; ++i) {
        float linear = static_cast<float>(i) / 255.0F;
        constexpr auto cutoff = 0.0031308F;
        linear = (linear <= cutoff)
                     ? (linear * 12.92F)
                     : (1.055F * std::powf(linear, 1.0F / 2.4F)) - 0.055F;
        lut[i] = static_cast<uint8_t>(
            std::clamp(
                static_cast<int>((linear * 255.0F) + 0.5F),
                0,
                255
            )
        );
    }
    return lut;
}

void decompress_dxt1(TextureExportInfo& out, uint8_t* data) {
    BlockDecompressImageDXT1(
        out.width,
        out.height,
        data,
        reinterpret_cast<unsigned long*>(out.data.get())
    );
}

void decompress_dxt5(TextureExportInfo& out, uint8_t* data) {
    BlockDecompressImageDXT5(
        out.width,
        out.height,
        data,
        reinterpret_cast<unsigned long*>(out.data.get())
    );
}

// NOLINTEND(*-pro-bounds-constant-array-index)

}  // namespace

}  // namespace world_exporter

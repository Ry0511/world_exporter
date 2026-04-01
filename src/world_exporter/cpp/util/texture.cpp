//
// Date       : 29/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/pch.h"
#include "world_exporter/cpp/util/texture.h"

#include "s3tc-dxt-decompression/s3tc.h"

namespace world_exporter::helpers {

namespace {

std::array<uint8_t, 256> srgb_lut_init() noexcept {
    // following: https://en.wikipedia.org/wiki/SRGB#Transfer_function_(%22gamma%22)
    // might as well use a look up table since it is not a whole lot of extra effort
    std::array<uint8_t, 256> lut{};
    for (int i = 0; i < 256; ++i) {
        float linear = static_cast<float>(i) / 255.0F;
        linear = (linear <= 0.04045F)
                     ? linear / 12.92F
                     : std::powf((linear + 0.055F) / 1.055F, 2.4F);
        // NOLINTNEXTLINE(*-pro-bounds-constant-array-index)
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

const std::array<uint8_t, 256> srgb_lut = srgb_lut_init();
}  // namespace

uint8_t to_linear(uint8_t val) noexcept {
    // NOLINTNEXTLINE(*-pro-bounds-constant-array-index)
    return srgb_lut[val];
}

void linear_abgr(uint32_t width, uint32_t height, uint8_t* data) noexcept {
    for (size_t i = 0; i < (width * height) * 4; i += 4) {
        data[i + 1] = to_linear(data[i + 1]);
        data[i + 2] = to_linear(data[i + 2]);
        data[i + 3] = to_linear(data[i + 3]);
    }
}

std::unique_ptr<uint8_t[]> decompress_dxt1(uint32_t width, uint32_t height, const uint8_t* data) {
    auto abgr = std::make_unique<uint8_t[]>(width * height * 4);
    BlockDecompressImageDXT1(width, height, data, reinterpret_cast<unsigned long*>(abgr.get()));
    return abgr;
}

std::unique_ptr<uint8_t[]> decompress_dxt5(uint32_t width, uint32_t height, const uint8_t* data) {
    auto abgr = std::make_unique<uint8_t[]>(width * height * 4);
    BlockDecompressImageDXT5(width, height, data, reinterpret_cast<unsigned long*>(abgr.get()));
    return abgr;
}

ExportedTexture export_texture_resource(FTexture2DResource* resource) {
    auto* handle = resource->TextureRhi.as<FD3D9Texture>();

    if (handle->Ref == nullptr) {
        throw std::runtime_error{"native handle is null"};
    }

    auto* texture = handle->Ref;
    IDirect3DTexture9* tex2d{nullptr};
    HRESULT result{0};

    result = texture->QueryInterface(IID_IDirect3DTexture9, reinterpret_cast<void**>(&tex2d));
    if (FAILED(result) || tex2d == nullptr) {
        throw std::runtime_error{"underlying texture is not a Texture2D"};
    }

    D3DSURFACE_DESC desc;
    result = tex2d->GetLevelDesc(0, &desc);
    if (FAILED(result)) {
        tex2d->Release();
        throw std::runtime_error{"failed to obtain surface zero description"};
    }

    constexpr std::array<D3DFORMAT, 4> supported_formats{
        D3DFMT_DXT1,
        D3DFMT_DXT5,
        D3DFMT_A8B8G8R8,
        D3DFMT_A8R8G8B8
    };
    if (
        !std::ranges::any_of(supported_formats, [&desc](D3DFORMAT l) { return l == desc.Format; })
    ) {
        tex2d->Release();
        throw std::runtime_error{
            std::format("unsupported pixel format - {}", static_cast<int>(desc.Format))
        };
    }

    IDirect3DSurface9* surface{nullptr};
    result = tex2d->GetSurfaceLevel(0, &surface);
    if (FAILED(result)) {
        tex2d->Release();
        throw std::runtime_error{"failed to obtain surface zero"};
    }

    D3DLOCKED_RECT locked_rect;
    result = surface->LockRect(&locked_rect, nullptr, D3DLOCK_READONLY);
    if (FAILED(result)) {
        surface->Release();
        tex2d->Release();
        throw std::runtime_error{"failed to lock surface zero"};
    }

    ExportedTexture out{
        desc.Width,
        desc.Height,
        std::make_unique<uint8_t[]>(desc.Width * desc.Height * 4)
    };

    // TODO: stride should be considered, currently assuming everything is packed
    if (desc.Format == D3DFMT_DXT1) {
        BlockDecompressImageDXT1(
            desc.Width,
            desc.Height,
            reinterpret_cast<const uint8_t*>(locked_rect.pBits),
            reinterpret_cast<unsigned long*>(out.data.get())
        );
    } else if (desc.Format == D3DFMT_DXT5) {
        BlockDecompressImageDXT5(
            desc.Width,
            desc.Height,
            reinterpret_cast<const uint8_t*>(locked_rect.pBits),
            reinterpret_cast<unsigned long*>(out.data.get())
        );
    } else if (desc.Format == D3DFMT_A8B8G8R8 || desc.Format == D3DFMT_A8R8G8B8) {
        std::memcpy(out.data.get(), locked_rect.pBits, desc.Width * desc.Height * 4);
    }

    surface->UnlockRect();
    surface->Release();
    tex2d->Release();

    // swizzle
    if (desc.Format == D3DFMT_A8R8G8B8) {
        for (size_t i = 0; i < (desc.Width * desc.Height) * 4; i += 4) {
            // TODO: eyeballed these
            std::swap(out.data[i + 0], out.data[i + 3]); // B <> A
            std::swap(out.data[i + 1], out.data[i + 2]); // G <> R
        }
    }

    if (handle->bSRGB != 0) {
        linear_abgr(desc.Width, desc.Height, out.data.get());
    }

    return out;
}

}  // namespace world_exporter::helpers
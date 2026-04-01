//
// Date       : 29/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_TEXTURE_H
#define WORLD_EXPORTER_TEXTURE_H

#include "world_exporter/cpp/pch.h"
#include "world_exporter/cpp/util/rhi.h"

namespace world_exporter::helpers {

////////////////////////////////////////////////////////////////////////////////
// | HARDWARE TEXTURE INFO |
////////////////////////////////////////////////////////////////////////////////

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

struct FTexture : FRenderResource {
    WORLD_EXPORTER_DISALLOW_CREATE(FTexture);
    TDynamicRHIResourceReference TextureRhi; // FD3D9Texture**
    TDynamicRHIResourceReference SamplerRhi; // ?
    uint8_t _3[36];
};

struct FTextureResource : FTexture {
    WORLD_EXPORTER_DISALLOW_CREATE(FTextureResource);
    uint8_t _3[4];
};

struct FTexture2DResource : FTextureResource {
    WORLD_EXPORTER_DISALLOW_CREATE(FTexture2DResource);
    unrealsdk::unreal::UObject* Owner;  // likely Texture2D
};

UNREALSDK_UNREAL_STRUCT_PADDING_POP()

////////////////////////////////////////////////////////////////////////////////
// | HELPER FUNCTIONS |
////////////////////////////////////////////////////////////////////////////////

// TODO
//  Standardise pixel data to be RGBA since thats what glTF expects
//  Normal textures are expected to be in the form RGB https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#additional-textures
//  Ambient, Roughness, and Metallic are packed into a single texture with R = Ambient, G = Roughness, and B = Metallic
//  For simplicity images should use either image/png or image/jpeg
//  Need to export the sampler settings i.e., min/mag filters and wrapping modes

struct ExportedTexture {
    uint32_t width;
    uint32_t height;
    std::unique_ptr<uint8_t[]> data;  // pixel data in the from ABGR
};

/**
 * Converts the input sRGB channel value to linear space
 * @param val sRGB channel value
 * @return linear space value
 */
uint8_t to_linear(uint8_t val) noexcept;

/**
 * Converts the input ABGR data to linear space
 * @param width the image width
 * @param height the image height
 * @param data Pointer to ABGR encoded data
 * @note performs in place mutation of the data
 * @note logically speaking only relevant factor is the index of the alpha channel
 */
void linear_abgr(uint32_t width, uint32_t height, uint8_t* data) noexcept;

/**
 * Decompresses pixel data compressed using DXT1 strategy
 * @param width the width of the input image
 * @param height the height of the input image
 * @param data the compressed image data
 * @return the uncompressed pixel data
 * @note https://en.wikipedia.org/wiki/S3_Texture_Compression
 */
std::unique_ptr<uint8_t[]> decompress_dxt1(uint32_t width, uint32_t height, const uint8_t* data);

/**
 * Decompresses pixel data compressed using DXT5 strategy
 * @param width the width of the input image
 * @param height the height of the input image
 * @param data the compressed image data
 * @return the uncompressed pixel data
 * @note https://en.wikipedia.org/wiki/S3_Texture_Compression
 */
std::unique_ptr<uint8_t[]> decompress_dxt5(uint32_t width, uint32_t height, const uint8_t* data);

/**
 * extracts from the texture resource the pixel data
 * @param resource the texture resource to extract
 * @return ABGR encoded pixel data
 * @note only DXT1, DXT5, and A8R8G8B8 formats are supported
 * @throws std::runtime_error on any invalid states
 */
ExportedTexture export_texture_resource(FTexture2DResource* resource);

}  // namespace world_exporter::helpers
#endif

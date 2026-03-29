//
// Date       : 22/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_MATERIALS_H
#define WORLD_EXPORTER_MATERIALS_H

#include "pyunrealsdk/pch.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/game/bl2/offsets.h"

#include "world_exporter/cpp/util/static_mesh.h"
#include "world_exporter/cpp/util/rhi.h"

namespace world_exporter {

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()
namespace helpers {
using namespace unrealsdk;

//
// Material defines how to obtain preset parameter values i.e.,
//   | Unreal                  | glTF                                              |
//   |-------------------------|---------------------------------------------------|
//   | Diffuse                 | Base Color                                        |
//   | DiffusePower            | Ignored                                           |
//   | Emissive                | Emissive Texture and Emissive Factor              |
//   | Specular                | Ignored (pbr/roughness but not exactly)           |
//   | SpecularPower           | Ignored                    ^^^^^^^^^^^            |
//   | Opacity                 | Alpha Mode & Alpha Cutoff                         |
//   | OpacityMask             | Alpha Mode & Alpha Cutoff                         |
//   | Distortion              | Ignored                                           |
//   | TransmissionMask        | Ignored                                           |
//   | TransmissionColor       | Ignored                                           |
//   | Normal                  | Normal & Normal Texture                           |
//   | CustomLighting          | Ignored                                           |
//   | CustomSkylightDiffuse   | Ignored                                           |
//
// Not all of the above need be given a value and we can't export everything.
//
//
//

struct FTextureMipBulkDataVftable {
    void* _1;
    void*(__thiscall* get_bulk_data)(void* self, void* texture, int index);
};

struct FTextureMipBulkData {
    FTextureMipBulkDataVftable* vftable;
    int32_t BulkDataFlags;
    int32_t ElementCount;
    int32_t BulkDataOffsetInFile;
    int32_t BulkDataSizeOnDisk;
    int32_t SavedBulkDataFlags;
    int32_t SavedElementCount;
    int32_t SavedBulkDataOffsetInFile;
    int32_t SavedBulkDataSizeOnDisk;
    uint8_t* BulkData;
    int32_t LockStatus;
    void* AttachedAr;
    int32_t bShouldFreeOnEmpty;
};

struct FTexture2DMipMap {
    FTextureMipBulkData Data;
    int32_t SizeX;
    int32_t SizeY;
};

struct FTexture : FRenderResource {
    TDynamicRHIResourceReference TextureRhi;
    TDynamicRHIResourceReference SamplerRhi;
    double LastRenderTime;
    float _2[5];
    uint8_t _3[8];
};

struct FTextureResource : FTexture {
    uint8_t _3[4];
};

struct FTexture2DResource : FTextureResource {
    unrealsdk::unreal::UObject* Owner;  // Texture2D
};

// 568BF1837E240075??8B068B5008538B5E0857FFD28B0D????????8BF80FAFFB85C975??E8????????8B0D????????8B018B50046A0857
constexpr unrealsdk::memory::Pattern<55> load_bulk_data{
    "56 8B F1 83 7E 24 00 75 ?? 8B 06 8B 50 08 53 8B 5E 08 57 FF D2 8B 0D ?? ?? ?? ?? 8B F8 0F AF FB 85 C9 75 ?? E8 ?? ?? ?? ?? 8B 0D ?? ?? ?? ?? 8B 01 8B 50 04 6A 08 57"
};

}  // namespace helpers
UNREALSDK_UNREAL_STRUCT_PADDING_POP()
}  // namespace world_exporter

#endif

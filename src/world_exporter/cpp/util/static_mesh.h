//
// Date       : 22/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_STATIC_MESH_H
#define WORLD_EXPORTER_STATIC_MESH_H

#include "pyunrealsdk/pch.h"
#include "unrealsdk/game/bl2/offsets.h"

#include "world_exporter/cpp/util/common.h"

namespace world_exporter {
namespace fs = std::filesystem;

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

namespace helpers {
using namespace unrealsdk;

////////////////////////////////////////////////////////////////////////////////
// | COMMON |
////////////////////////////////////////////////////////////////////////////////

struct TStaticMeshFullVertexFloat16UVs {
    FPackedNormal TangentX;
    FPackedNormal TangentZ;
    // variable size - determined by ::NumTexCoords
    FVector2DHalf UVs[4];
};

struct FFragmentRange {
    int32_t BaseIndex;
    int32_t NumPrimitives;
};

struct FRenderResource {
    uintptr_t* vftable;
    uint32_t _1[3];
    uint32_t bIsInitialised : 1;
};

struct FVertexBuffer : FRenderResource {
    uint32_t _1;
};

struct FStaticMeshVertexDataInterface {
    struct Vftable {
        void* _1;
        void* _2;
        uint32_t(__thiscall* get_stride)(void* self);
        uint8_t*(__thiscall* get_data_ptr)(void* self);
    };
    Vftable* vftable;
    auto stride() { return vftable->get_stride(this); }
    auto data() { return vftable->get_data_ptr(this); }
};

template <class T>
struct TStaticMeshVertexData : FStaticMeshVertexDataInterface, TResourceArray<T> {};

////////////////////////////////////////////////////////////////////////////////
// | FStaticMeshVertexBuffer |
////////////////////////////////////////////////////////////////////////////////

struct FStaticMeshVertexBuffer : FVertexBuffer {
    FStaticMeshVertexDataInterface* VertexData;
    uint32_t NumTexCoords;
    uint8_t* Data;
    uint32_t Stride;
    uint32_t NumVertices;
    uint32_t bUseFullPrecisionUVs;
};

////////////////////////////////////////////////////////////////////////////////
// | FPositionVertexBuffer |
////////////////////////////////////////////////////////////////////////////////

struct FPositionVertexData : TStaticMeshVertexData<FVector> {};

struct FPositionVertexBuffer : FVertexBuffer {
    FPositionVertexData* VertexData;
    uint8_t* Data;
    uint32_t Stride;
    uint32_t NumVertices;
};

////////////////////////////////////////////////////////////////////////////////
// | FColourVertexBuffer |
////////////////////////////////////////////////////////////////////////////////

struct FColorVertexData : TStaticMeshVertexData<FColor> {};

struct FColourVertexBuffer : FVertexBuffer {
    FColorVertexData* VertexData;
    uint8_t* Data;
    uint32_t Stride;
    uint32_t NumVertices;
};

////////////////////////////////////////////////////////////////////////////////
// | FRawStaticIndexBuffer |
////////////////////////////////////////////////////////////////////////////////

struct FIndexBuffer : FRenderResource {
    uint32_t _1;
};

struct FRawStaticIndexBuffer : FIndexBuffer {
    unreal::TArray<int16_t> Indices;
    uint32_t NumVertsPerInstance;
    uint32_t PreallocateInstanceCount;
    uint32_t bSetupForInstancing;
};

////////////////////////////////////////////////////////////////////////////////
// | FStaticMeshLodInfo |
////////////////////////////////////////////////////////////////////////////////

struct FStaticMeshLodElement {
    void* Material;
    uint32_t bEnableShadowCasting;
    uint32_t bSelected;
    int32_t bEnableCollision : 1;
};

struct FStaticMeshLodInfo {
    unreal::TArray<FStaticMeshLodElement> Elements;
};

////////////////////////////////////////////////////////////////////////////////
// | CONTAINERS |
////////////////////////////////////////////////////////////////////////////////

struct FStaticMeshElement {
    void* Material;
    unreal::UnmanagedFString Name;
    int32_t bEnableCollision;
    int32_t bOldEnableCollision;
    int32_t bEnableShadowCasting;
    uint32_t FirstIndex;
    uint32_t NumTriangles;
    uint32_t MinVertexIndex;
    uint32_t MaxVertexIndex;
    uint32_t MaterialIndex;
    unreal::TArray<FFragmentRange> Fragments;
    void* PlatformData;
};

struct FStaticMeshRenderData {
    FStaticMeshVertexBuffer VertexBuffer;
    FPositionVertexBuffer PositionVertexBuffer;
    FColourVertexBuffer ColourVertexBuffer;
    uint32_t NumVertices;
    uint32_t bNeedsCpuAccess;
    FRawStaticIndexBuffer IndexBuffer;
    uint8_t _1[36];
    unreal::TArray<FStaticMeshElement> SubMeshes;
};

struct UStaticMesh : game::bl2::UObject {
    unreal::TArray<FStaticMeshRenderData*> LodModels;
    unreal::TArray<FStaticMeshLodInfo> LodInfo;
    float LodDistanceRatio;
    float LodMaxRange;
};

}

UNREALSDK_UNREAL_STRUCT_PADDING_POP()
}  // namespace world_exporter
#endif

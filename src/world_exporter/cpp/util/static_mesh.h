//
// Date       : 22/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_STATIC_MESH_H
#define WORLD_EXPORTER_STATIC_MESH_H

#include "world_exporter/cpp/pch.h"
#include "unrealsdk/game/bl2/offsets.h"

#include "world_exporter/cpp/util/common.h"

namespace world_exporter {

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

namespace helpers {
using namespace unrealsdk;

////////////////////////////////////////////////////////////////////////////////
// | COMMON |
////////////////////////////////////////////////////////////////////////////////

constexpr size_t MAX_UV_COUNT = 4;

// TODO: at somepoint will want to look into 32bit uvs however I have not seen them be used at all
//  in bl2 thus far
struct TStaticMeshFullVertexFloat16UVs {
    WORLD_EXPORTER_DISALLOW_CREATE(TStaticMeshFullVertexFloat16UVs);
    FPackedNormal TangentX;
    FPackedNormal TangentZ;
    // variable size - determined by ::NumTexCoords
    FVector2DHalf UVs[MAX_UV_COUNT];
};

struct FFragmentRange {
    WORLD_EXPORTER_DISALLOW_CREATE(FFragmentRange);
    int32_t BaseIndex;
    int32_t NumPrimitives;
};

struct FRenderResource {
    WORLD_EXPORTER_DISALLOW_CREATE(FRenderResource);
    uintptr_t* vftable;
    uint32_t _1[4];
};

struct FVertexBuffer : FRenderResource {
    WORLD_EXPORTER_DISALLOW_CREATE(FVertexBuffer);
    uint32_t _1;
};

struct FStaticMeshVertexDataInterface {
    WORLD_EXPORTER_DISALLOW_CREATE(FStaticMeshVertexDataInterface);
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
struct TStaticMeshVertexData : FStaticMeshVertexDataInterface, TResourceArray<T> {
    WORLD_EXPORTER_DISALLOW_CREATE(TStaticMeshVertexData);
};

////////////////////////////////////////////////////////////////////////////////
// | FStaticMeshVertexBuffer |
////////////////////////////////////////////////////////////////////////////////

struct FStaticMeshVertexBuffer : FVertexBuffer {
    WORLD_EXPORTER_DISALLOW_CREATE(FStaticMeshVertexBuffer);
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

struct FPositionVertexData : TStaticMeshVertexData<FVector> {
    WORLD_EXPORTER_DISALLOW_CREATE(FPositionVertexData);
};

struct FPositionVertexBuffer : FVertexBuffer {
    WORLD_EXPORTER_DISALLOW_CREATE(FPositionVertexBuffer);
    FPositionVertexData* VertexData;
    uint8_t* Data;
    uint32_t Stride;
    uint32_t NumVertices;
};

////////////////////////////////////////////////////////////////////////////////
// | FColourVertexBuffer |
////////////////////////////////////////////////////////////////////////////////

struct FColorVertexData : TStaticMeshVertexData<FColor> {
    WORLD_EXPORTER_DISALLOW_CREATE(FColorVertexData);
};

struct FColourVertexBuffer : FVertexBuffer {
    WORLD_EXPORTER_DISALLOW_CREATE(FColourVertexBuffer);
    FColorVertexData* VertexData;
    uint8_t* Data;
    uint32_t Stride;
    uint32_t NumVertices;
};

////////////////////////////////////////////////////////////////////////////////
// | FRawStaticIndexBuffer |
////////////////////////////////////////////////////////////////////////////////

struct FIndexBuffer : FRenderResource {
    WORLD_EXPORTER_DISALLOW_CREATE(FIndexBuffer);
    uint32_t _1;
};

struct FRawStaticIndexBuffer : FIndexBuffer {
    WORLD_EXPORTER_DISALLOW_CREATE(FRawStaticIndexBuffer);
    unreal::TArray<int16_t> Indices;
    uint32_t NumVertsPerInstance;
    uint32_t PreallocateInstanceCount;
    uint32_t bSetupForInstancing;
};

////////////////////////////////////////////////////////////////////////////////
// | FStaticMeshLodInfo |
////////////////////////////////////////////////////////////////////////////////

struct FStaticMeshLodElement {
    WORLD_EXPORTER_DISALLOW_CREATE(FStaticMeshLodElement);
    unreal::UObject* Material;
    uint32_t bEnableShadowCasting;
    uint32_t bSelected;
    int32_t bEnableCollision : 1;
};

struct FStaticMeshLodInfo {
    WORLD_EXPORTER_DISALLOW_CREATE(FStaticMeshLodInfo);
    unreal::TArray<FStaticMeshLodElement> Elements;
};

////////////////////////////////////////////////////////////////////////////////
// | CONTAINERS |
////////////////////////////////////////////////////////////////////////////////

struct FStaticMeshElement {
    WORLD_EXPORTER_DISALLOW_CREATE(FStaticMeshElement);
    void* Material; // UMaterialInterface
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
    WORLD_EXPORTER_DISALLOW_CREATE(FStaticMeshRenderData);
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
    WORLD_EXPORTER_DISALLOW_CREATE(UStaticMesh);
    unreal::TArray<FStaticMeshRenderData*> LodModels;
    unreal::TArray<FStaticMeshLodInfo> LodInfo;
    float LodDistanceRatio;
    float LodMaxRange;
};

}  // namespace helpers

UNREALSDK_UNREAL_STRUCT_PADDING_POP()
}  // namespace world_exporter
#endif

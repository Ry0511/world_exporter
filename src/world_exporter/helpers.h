//
// Date       : 07/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#ifndef WORLD_EXPORTER_HELPERS_H
#define WORLD_EXPORTER_HELPERS_H

#include "pyunrealsdk/pch.h"
#include "unrealsdk/game/bl2/offsets.h"

namespace world_exporter::helpers {
UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

using namespace unrealsdk;
using namespace unrealsdk::unreal;

template <class T>
struct TArrayWithOwner : TArray<T> {
    game::bl2::UObject* Owner;
};

struct FVector {
    float X, Y, Z;
};

struct FPlane : FVector {
    float W;
};

struct FRotator {
    int Pitch, Yaw, Roll;
};

struct FURL {
    UnmanagedFString Protocol;
    UnmanagedFString Host;
    int32_t Port;
    UnmanagedFString Map;
    TArray<UnmanagedFString> Op;
    UnmanagedFString Portal;
    bool Valid;
};

struct FStaticMeshLodElement {
    void* Material;
    bool bEnableShadowCasting;
    bool bSelected;
    int32_t bEnableCollision : 1;
};

struct FStaticMeshLodInfo {
    TArray<FStaticMeshLodElement> Elements;
};

struct FVertexBuffer {
    uintptr_t* vftable;
    uint32_t _1[3]; // resource linked list
    bool bInitialised;
    uint32_t _2; // native renderer handle?
};

struct FStaticMeshVertexBuffer : FVertexBuffer {
    uint8_t* VertexData;
    uint32_t NumTexCoords;
    uint8_t* Data;
    uint32_t Stride;
    uint32_t NumVertices;
    uint32_t bUseFullPrecisionUVs;
};

struct FPositionVertexBuffer : FVertexBuffer {
    void* VertexData;
    uint8_t* Data;
    uint32_t Stride;
    uint32_t NumVertices;
};

struct FColourVertexBuffer : FVertexBuffer {
    void* VertexData;
    uint8_t* Data;
    uint32_t Stride;
    uint32_t NumVertices;
};

struct FFragmentRange {
    int32_t BaseIndex;
    int32_t NumPrimitives;
};

struct FStaticMeshElement {
    void* Material;
    UnmanagedFString Name;
    int32_t bEnableCollision;
    int32_t bOldEnableCollision;
    int32_t bEnableShadowCasting;
    uint32_t FirstIndex;
    uint32_t NumTriangles;
    uint32_t MinVertexIndex;
    uint32_t MaxVertexIndex;
    uint32_t MaterialIndex;
    TArray<FFragmentRange> Fragments;
    void* PlatformData;
};

struct FStaticMeshRenderData {
    FStaticMeshVertexBuffer VertexBuffer;
    FPositionVertexBuffer PositionVertexBuffer;
    FColourVertexBuffer ColourVertexBuffer;
    uint32_t NumVertices;
    uint8_t _1[88];
    TArray<FStaticMeshElement> SubMeshes;
};

struct UStaticMesh : game::bl2::UObject {
    TArray<FStaticMeshRenderData*> LodModels;
    TArray<FStaticMeshLodInfo> LodInfo;
    float LodDistanceRatio;
    float LodMaxRange;
};

UNREALSDK_UNREAL_STRUCT_PADDING_POP()
}  // namespace world_exporter::helpers

#endif

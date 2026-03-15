//
// Date       : 07/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#ifndef WORLD_EXPORTER_HELPERS_H
#define WORLD_EXPORTER_HELPERS_H

#include "pyunrealsdk/pch.h"
#include "unrealsdk/game/bl2/offsets.h"

namespace world_exporter {
UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

namespace fs = std::filesystem;

// TODO: At some point all of this is going to need to be cleand up

namespace helpers {
using namespace unrealsdk;

template <class T>
struct TArrayWithOwner : unreal::TArray<T> {
    unreal::UObject* Owner;
};

template <class T>
struct TResourceArray {
    void* vftable;
    unreal::TArray<T> Data;
    uint32_t bNeedsCpuAccess;
};

struct Actor : game::bl2::UObject {
    uint8_t _1[328];
};

template <class T, auto SmallBufferSize>
struct TArrayInline {
    T InlineData[SmallBufferSize];
    T* SecondaryData;
    int32_t Length;
    int32_t Count;

    T* at(size_t i) {
        if (Count < SmallBufferSize) {
            return InlineData[i];
        }
        return SecondaryData[i];
    }

    T& operator[](size_t i) {
        return *at(i);
    }
};

struct FVector {
    float X, Y, Z;
};

struct FVector2D {
    float X, Y;
};

struct FVector4 {
    float X, Y, Z, W;
};

struct FPlane : FVector {
    float W;
};

struct FRotator {
    int Pitch, Yaw, Roll;
};

struct FMatrix {
    float M[4][4];
};

struct FColor {
    uint8_t B, G, R, A;
};

// for reference
// https://en.wikipedia.org/wiki/IEEE_754
// https://en.wikipedia.org/wiki/Half-precision_floating-point_format
// https://en.wikipedia.org/wiki/Single-precision_floating-point_format
//
struct Float32 {
    union {
        struct {
            uint32_t Mantissa : 23;
            uint32_t Exponent : 8;
            uint32_t Sign : 1;
        } V;
        float32_t FloatValue;
    };
};

struct Float16 {
    union {
        struct {
            uint16_t Mantissa : 10;
            uint16_t Exponent : 5;
            uint16_t Sign : 1;
        } V;
        uint16_t Raw;
    };

    float as_float() const noexcept {
        Float32 ret{};
        ret.V.Sign = V.Sign;

        if (V.Exponent == 0) {
            ret.V.Exponent = 0;
            ret.V.Mantissa = 0;
        } else if (V.Exponent == 31) {
            ret.V.Exponent = 142;
            ret.V.Mantissa = 8380416;
        } else {
            ret.V.Exponent = int32_t(V.Exponent) - 15 + 127;
            ret.V.Mantissa = int16_t(V.Mantissa) << 13;
        }

        return ret.FloatValue;
    };
};

struct FVector2DHalf {
    Float16 X, Y;

    FVector2D as_vec2() const noexcept {
        return FVector2D{X.as_float(), Y.as_float()};
    }
};

struct FPackedNormal {
    uint8_t W, Z, Y, X;
};

// variable struct size actual size determined by stride
struct TStaticMeshFullVertexFloat16UVs {
    FPackedNormal TangentX;
    FPackedNormal TangentZ;
    // variable size - determined by ::NumTexCoords
    FVector2DHalf UVs[4];
};

struct FURL {
    unreal::UnmanagedFString Protocol;
    unreal::UnmanagedFString Host;
    int32_t Port;
    unreal::UnmanagedFString Map;
    unreal::TArray<unreal::UnmanagedFString> Op;
    unreal::UnmanagedFString Portal;
    bool Valid;
};

// TODO: Needs to be validated haven't seen anything actually populate this
struct FStaticMeshLodElement {
    void* Material;
    uint32_t bEnableShadowCasting;
    uint32_t bSelected;
    int32_t bEnableCollision : 1;
};

struct FStaticMeshLodInfo {
    unreal::TArray<FStaticMeshLodElement> Elements;
};

struct FRenderResource {
    uintptr_t* vftable;
    uint32_t _1[3];
    uint32_t bIsInitialised : 1;
};

struct FIndexBuffer : FRenderResource {
    uint32_t _1;
};

struct FRawStaticIndexBuffer : FIndexBuffer {
    unreal::TArray<int16_t> Indices;
    uint32_t NumVertsPerInstance;
    uint32_t PreallocateInstanceCount;
    uint32_t bSetupForInstancing;
};

struct FVertexBuffer : FRenderResource {
    uint32_t _1;
};

struct FStaticMeshVertexDataInterface {
    struct Vftable {
        void* _1;
        void* _2;
        // basically always implemented as `return 12;`
        uint32_t(__thiscall* get_stride)(void* self);
        uint8_t*(__thiscall* get_data_ptr)(void* self);
    };
    Vftable* vftable;
    auto stride() { return vftable->get_stride(this); }
    auto data() { return vftable->get_data_ptr(this); }
};

struct FStaticMeshVertexBuffer : FVertexBuffer {
    FStaticMeshVertexDataInterface* VertexData;
    uint32_t NumTexCoords;
    uint8_t* Data;
    uint32_t Stride;
    uint32_t NumVertices;
    uint32_t bUseFullPrecisionUVs;
};

template <class T>
struct TStaticMeshVertexData : FStaticMeshVertexDataInterface, TResourceArray<T> {};

struct FPositionVertexData : TStaticMeshVertexData<FVector> {};

struct FPositionVertexBuffer : FVertexBuffer {
    FPositionVertexData* VertexData;
    uint8_t* Data;
    uint32_t Stride;
    uint32_t NumVertices;
};

struct FColorVertexData : TStaticMeshVertexData<FColor> {};

struct FColourVertexBuffer : FVertexBuffer {
    FColorVertexData* VertexData;
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

struct FModelVertex {
    FVector Position;
    FPackedNormal TangentX;
    FPackedNormal TangentZ;
    FVector2D TexCoord;
    FVector2D ShadowTexCoord;
};

struct FPoly {
    FVector Base;
    FVector Normal;
    FVector TextureU;
    FVector TextureV;
    TArrayInline<FVector, 16> Vertices;
};

struct UPolys : game::bl2::UObject {
    TArrayWithOwner<FPoly> Elements;
};

struct FVert {
    int32_t VertexIndex;
    int32_t Side;
    FVector2D ShadowTexCoord;
    FVector2D BackfaceShadowTexCoord;
};

struct UModel : game::bl2::UObject {
    UPolys* Polys;
    TArrayWithOwner<void*> _1;
    TArrayWithOwner<FVert> Verts;
    TArrayWithOwner<FVector> Vectors;
    TArrayWithOwner<FVector> Points;
    TArrayWithOwner<void*> _2;
};

struct ULevel : game::bl2::UObject {
    TArrayWithOwner<unreal::UObject*> Actors;
    FURL Url;
    UModel* Model;
    unreal::TArray<void*> ModelComponents;
    unreal::TArray<void*> GameSequences;
};

struct UWorld : game::bl2::UObject {
    uintptr_t* FNetworkNotifyVtable;
    void* FSceneInterface;
    unreal::TArray<ULevel*> Levels;
    ULevel* PersistentLevel;
    void* PersistentFaceFxAnimSet;
    ULevel* CurrentLevel;
    ULevel* CurrentLevelPendingVisibility;
};

struct StaticMeshComponent : game::bl2::UObject {
    uint8_t _1[476];
    UStaticMesh* StaticMesh;  // 540
};

constexpr FVector ZERO_VECTOR = FVector{0.0F, 0.0F, 0.0F};
FVector transform_point(const FMatrix& mat, const FVector& point);

}  // namespace helpers

UNREALSDK_UNREAL_STRUCT_PADDING_POP()
}  // namespace world_exporter

#endif

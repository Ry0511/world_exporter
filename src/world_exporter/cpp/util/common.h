//
// Date       : 22/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_COMMON_H
#define WORLD_EXPORTER_COMMON_H

#include "pyunrealsdk/pch.h"
#include "unrealsdk/game/bl2/offsets.h"

namespace world_exporter {

namespace fs = std::filesystem;

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()
namespace helpers {

using namespace unrealsdk;

////////////////////////////////////////////////////////////////////////////////
// | ARRAY TYPES |
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// | MATHS TYPES |
////////////////////////////////////////////////////////////////////////////////

// clang-format off
struct FVector          { float X, Y, Z;        };
struct FVector2D        { float X, Y;           };
struct FVector4         { float X, Y, Z, W;     };
struct FPlane : FVector { float W;              };
struct FRotator         { int Pitch, Yaw, Roll; };
struct FMatrix          { float M[4][4];        };
struct FColor           { uint8_t B, G, R, A;   };
// clang-format on

////////////////////////////////////////////////////////////////////////////////
// | FLOAT16 & FLOAT32 |
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// |  |
////////////////////////////////////////////////////////////////////////////////

struct FURL {
    unreal::UnmanagedFString Protocol;
    unreal::UnmanagedFString Host;
    int32_t Port;
    unreal::UnmanagedFString Map;
    unreal::TArray<unreal::UnmanagedFString> Op;
    unreal::UnmanagedFString Portal;
    bool Valid;
};

struct Actor : game::bl2::UObject {
    uint8_t _1[328];
};

}  // namespace helpers
UNREALSDK_UNREAL_STRUCT_PADDING_POP()

}  // namespace world_exporter

#endif

//
// Date       : 28/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_RHI_H
#define WORLD_EXPORTER_RHI_H

#include "world_exporter/cpp/pch.h"
#include "world_exporter/cpp/util/static_mesh.h"

namespace world_exporter {
UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

namespace helpers {
using namespace unrealsdk;

struct TDynamicRHIResource {};

// proxy for an implementation defined handle
struct TDynamicRHIResourceReference {
    TDynamicRHIResource* Reference;
    template <class T>
    T* as() const noexcept { return static_cast<T*>(Reference); }
};

struct FRefCountedObject {
    int NumRefs;
};

template <class T>
struct TRefCountPtr {
    T* Ref;
};

template<class D3DTextureType>
struct TD3D9Texture : FRefCountedObject, TRefCountPtr<D3DTextureType>, TDynamicRHIResource {
    int32_t PixelFormat; // Texture::EPixelFormat 2,5,7 are relevant
    int32_t MemorySize;
    uint32_t bSRGB : 1;
    uint32_t bDynamic : 1;
};

using FD3D9Texture = TD3D9Texture<IDirect3DBaseTexture9>;

};  // namespace helpers

UNREALSDK_UNREAL_STRUCT_PADDING_POP()
}  // namespace world_exporter

#endif
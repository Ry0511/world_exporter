//
// Date       : 22/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_WORLD_H
#define WORLD_EXPORTER_WORLD_H

#include "pyunrealsdk/pch.h"
#include "unrealsdk/game/bl2/offsets.h"

#include "world_exporter/cpp/util/common.h"

namespace world_exporter {
namespace fs = std::filesystem;

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()
namespace helpers {
using namespace unrealsdk;

struct ULevel : game::bl2::UObject {
    TArrayWithOwner<unreal::UObject*> Actors;
    FURL Url;
    void* Model;
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

}
UNREALSDK_UNREAL_STRUCT_PADDING_POP()
}  // namespace world_exporter

#endif

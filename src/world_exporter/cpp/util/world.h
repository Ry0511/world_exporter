//
// Date       : 22/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_WORLD_H
#define WORLD_EXPORTER_WORLD_H

#include "world_exporter/cpp/pch.h"
#include "unrealsdk/game/bl2/offsets.h"

#include "world_exporter/cpp/util/common.h"

namespace world_exporter {

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()
namespace helpers {
using namespace unrealsdk;

//
// Some helpers which I am not sure if they will ever be used. Perhaps at somepoint we will split
// out objects into scenes based on levels but it depends on how much information is captured in a
// level. I have seen certain levels be dedicated to sound which likely don't contain anything visually.
//

struct ULevel : game::bl2::UObject {
    WORLD_EXPORTER_DISALLOW_CREATE(ULevel);
    TArrayWithOwner<unreal::UObject*> Actors;
    FURL Url;
    void* Model;
    unreal::TArray<void*> ModelComponents;
    unreal::TArray<void*> GameSequences;
};

struct UWorld : game::bl2::UObject {
    WORLD_EXPORTER_DISALLOW_CREATE(UWorld);
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

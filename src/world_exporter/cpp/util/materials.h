//
// Date       : 22/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_MATERIALS_H
#define WORLD_EXPORTER_MATERIALS_H

#include "pyunrealsdk/pch.h"

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

// TODO: Extract material graph

}  // namespace helpers
UNREALSDK_UNREAL_STRUCT_PADDING_POP()
}  // namespace world_exporter

#endif

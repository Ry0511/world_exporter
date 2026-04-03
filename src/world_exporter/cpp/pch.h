//
// Date       : 29/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_PCH_H
#define WORLD_EXPORTER_PCH_H

////////////////////////////////////////////////////////////////////////////////
// | DX9 AND WINDOWS |
////////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#define WIN32_NO_STATUS
#define NOMINMAX
#include <windows.h>
#undef ERROR  // conflicts with logging macro

#include <d3d9.h>

////////////////////////////////////////////////////////////////////////////////
// | HELPER MACROS |
////////////////////////////////////////////////////////////////////////////////

// boilerplate generator for a class/struct that should not be constructed, copied or destroyed
#define WORLD_EXPORTER_DISALLOW_CREATE(type) \
   public:                                   \
    type() = delete;                         \
    ~type() = delete;                        \
    type(const type&) = delete;              \
    type(type&&) = delete;                   \
    type& operator=(const type&) = delete;   \
    type& operator=(type&&) = delete

////////////////////////////////////////////////////////////////////////////////
// | CPP STUFF |
////////////////////////////////////////////////////////////////////////////////

#include <random>

#include "pyunrealsdk/pch.h"
#include "tinygltf/tiny_gltf.h"

// slightly less flexibility here but we don't really care too much about that
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace fs = std::filesystem;

#endif

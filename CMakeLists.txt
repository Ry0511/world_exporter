cmake_minimum_required(VERSION 3.25)
project(world_exporter VERSION 1.0.0)

# TODO: this file is quite messy and hard to follow - just like all cmake files but it doesn't have to be

set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
add_subdirectory(vendor/tinygltf EXCLUDE_FROM_ALL)
add_subdirectory(vendor/pyunrealsdk EXCLUDE_FROM_ALL)

################################################################################
# | WORLD EXPORTER TARGET |
################################################################################

file(GLOB_RECURSE WORLD_EXPORTER_SOURCES "${PROJECT_SOURCE_DIR}/src/world_exporter/cpp/*.cpp")
message(STATUS "world_exporter source files ${WORLD_EXPORTER_SOURCES}")

pyunrealsdk_add_module(world_exporter ${WORLD_EXPORTER_SOURCES})
target_include_directories(world_exporter PUBLIC ./src)
target_include_directories(world_exporter SYSTEM PRIVATE "vendor")
target_precompile_headers(world_exporter PRIVATE "${PROJECT_SOURCE_DIR}/src/world_exporter/cpp/pch.h")

install(
    TARGETS world_exporter
    RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/src/world_exporter/py"
    LIBRARY DESTINATION "${PROJECT_SOURCE_DIR}/src/world_exporter/py"
    ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/src/world_exporter/py"
)

install(
    FILES $<TARGET_PDB_FILE:world_exporter>
    DESTINATION "${PROJECT_SOURCE_DIR}/src/world_exporter/py"
    OPTIONAL
)

set(GLM_ENABLE_CXX_20 ON CACHE INTERNAL "" FORCE)
add_subdirectory(vendor/glm EXCLUDE_FROM_ALL)
target_link_libraries(world_exporter PRIVATE glm::glm)
target_link_libraries(world_exporter PRIVATE dxguid.lib)

target_sources(world_exporter PRIVATE "${PROJECT_SOURCE_DIR}/vendor/s3tc-dxt-decompression/s3tc.cpp")

################################################################################
# | TINYGLTF TEST | TODO: delete this
################################################################################

add_executable(tinygltftest "./src/world_exporter/other/tinygltftest.cpp")
target_include_directories(tinygltftest PRIVATE "vendor")
set_target_properties(
    tinygltftest
    PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

################################################################################
# | PLAYGROUND TARGET |
################################################################################

add_executable(playground "${PROJECT_SOURCE_DIR}/src/world_exporter/other/test.cpp")
target_include_directories(playground PUBLIC ./src)
target_include_directories(playground SYSTEM PRIVATE "vendor")
target_link_libraries(playground PRIVATE glm::glm)
set_target_properties(
    playground
    PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)